#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameplayTypes/CombatTypes.h"
#include "GameplayTypes/AbilityTypesLibrary.h"

void UUnitAbilityInstance::InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner)
{
	Config = InDefinition;
	Owner = InOwner;
	if (Config)
	{
		RemainingCharges = Config->MaxCharges;
	}
}
FAbilityResult UUnitAbilityInstance::TriggerAbility(AUnit* SourceUnit, FTacCoordinates TargetCell)
{
	FAbilityValidation Validation = CanExecute(SourceUnit, TargetCell);
	if (!Validation.bIsValid)
	{
		return CreateFailureResult(Validation.FailureReason, Validation.FailureMessage);
	}
	return CreateSuccessResult();
}

FAbilityResult UUnitAbilityInstance::ApplyAbilityEffect(AUnit* SourceUnit, FTacCoordinates TargetCell)
{
	return CreateSuccessResult();
}
ETargetReach UUnitAbilityInstance::GetTargeting() const
{
	if (Config)
	{
		return Config->Targeting;
	}
	return ETargetReach::None;
}
bool UUnitAbilityInstance::IsPassive() const
{
	if (Config)
	{
		return Config->bIsPassive;
	}
	return false;
}
TMap<AUnit*, FPreviewHitResult> UUnitAbilityInstance::DamagePreview(AUnit* Source, const TArray<AUnit*>& Targets)
{
	return TMap<AUnit*, FPreviewHitResult>();
}
void UUnitAbilityInstance::Subscribe()
{
	if (!Owner)
	{
		return;
	}
	Owner->OnUnitAttacked.AddDynamic(this, &UUnitAbilityInstance::HandleUnitAttacked);
	Owner->OnUnitDamaged.AddDynamic(this, &UUnitAbilityInstance::HandleUnitDamaged);
	Owner->OnUnitAttacks.AddDynamic(this, &UUnitAbilityInstance::HandleUnitAttacks);
}
void UUnitAbilityInstance::Unsubscribe()
{
	if (!Owner)
	{
		return;
	}
	Owner->OnUnitAttacked.RemoveDynamic(this, &UUnitAbilityInstance::HandleUnitAttacked);
	Owner->OnUnitDamaged.RemoveDynamic(this, &UUnitAbilityInstance::HandleUnitDamaged);
	Owner->OnUnitAttacks.RemoveDynamic(this, &UUnitAbilityInstance::HandleUnitAttacks);
}
void UUnitAbilityInstance::HandleUnitAttacked(AUnit* Victim, AUnit* Attacker)
{
}
void UUnitAbilityInstance::HandleUnitDamaged(AUnit* Victim, AUnit* Attacker)
{
}
void UUnitAbilityInstance::HandleUnitAttacks(AUnit* Attacker, AUnit* Target)
{
}
void UUnitAbilityInstance::ConsumeCharge()
{
	if (RemainingCharges > 0)
	{
		RemainingCharges--;
	}
}
void UUnitAbilityInstance::RestoreCharges()
{
	if (Config)
	{
		RemainingCharges = Config->MaxCharges;
	}
}
FAbilityDisplayData UUnitAbilityInstance::GetAbilityDisplayData() const
{
	FAbilityDisplayData DisplayData;
	if (!Config)
	{
		return DisplayData;
	}
	DisplayData.AbilityName = Config->AbilityName;
	DisplayData.Icon = Config->Icon;
	DisplayData.RemainingCharges = RemainingCharges;
	DisplayData.MaxCharges = Config->MaxCharges;
	DisplayData.bIsEmpty = false;
	if (Config->MaxCharges < 0)
	{
		DisplayData.bCanExecuteThisTurn = true;
	}
	else
	{
		DisplayData.bCanExecuteThisTurn = RemainingCharges > 0;
	}
	DisplayData.TargetingInfo = UAbilityTypesLibrary::TargetReachToString(Config->Targeting);
	DisplayData.Description = FString::Printf(TEXT("%s - %s"), *Config->AbilityName, *DisplayData.TargetingInfo);
	return DisplayData;
}
FAbilityValidation UUnitAbilityInstance::CanExecute(AUnit* SourceUnit, FTacCoordinates TargetCell) const
{
	if (Config && Config->MaxCharges > 0)
	{
		if (RemainingCharges <= 0)
		{
			return FAbilityValidation::Failure(EAbilityFailureReason::NoCharges,
				FText::FromString("No charges remaining"));
		}
	}

	// Target validation moved to derived classes - they know their targeting rules
	return FAbilityValidation::Success();
}
FAbilityResult UUnitAbilityInstance::CreateSuccessResult() const
{
	FAbilityResult Result = FAbilityResult::Success(TurnAction);
	return Result;
}
FAbilityResult UUnitAbilityInstance::CreateFailureResult(EAbilityFailureReason Reason, FText Message) const
{
	return FAbilityResult::Failure(Reason, Message);
}

UTacGridSubsystem* UUnitAbilityInstance::GetGridSubsystem(AUnit* Unit) const
{
	if (!Unit) return nullptr;
	UWorld* World = Unit->GetWorld();
	if (!World) return nullptr;
	return World->GetSubsystem<UTacGridSubsystem>();
}

UTacGridMovementService* UUnitAbilityInstance::GetMovementService(AUnit* Unit) const
{
	UTacGridSubsystem* GridSys = GetGridSubsystem(Unit);
	return GridSys ? GridSys->GetGridMovementService() : nullptr;
}

UTacGridTargetingService* UUnitAbilityInstance::GetTargetingService(AUnit* Unit) const
{
	UTacGridSubsystem* GridSys = GetGridSubsystem(Unit);
	return GridSys ? GridSys->GetGridTargetingService() : nullptr;
}

UTacCombatSubsystem* UUnitAbilityInstance::GetCombatSubsystem(AUnit* Unit) const
{
	if (!Unit) return nullptr;
	UWorld* World = Unit->GetWorld();
	if (!World) return nullptr;

	// TODO: Get from world subsystem once TacCombatSubsystem becomes UWorldSubsystem
	// Currently TacCombatSubsystem is UObject, need to access via GridSubsystem or similar
	return nullptr;
}


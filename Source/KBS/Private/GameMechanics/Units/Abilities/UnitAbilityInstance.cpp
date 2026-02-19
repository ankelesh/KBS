#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
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

bool UUnitAbilityInstance::RefreshAvailability() const
{
	bool bResult = CanExecute();
	OnAbilityAvailabilityChange.Broadcast(this, bResult);
	return bResult;
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

void UUnitAbilityInstance::HandleTurnEnd()
{
	RestoreCharges();
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

void UUnitAbilityInstance::BroadcastUsage() const
{
	OnAbilityUsed.Broadcast(RemainingCharges, CanExecute());
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
	DisplayData.bCanExecuteThisTurn = CanExecute();
	DisplayData.TargetingInfo = UAbilityTypesLibrary::TargetReachToString(Config->Targeting);
	DisplayData.Description = FString::Printf(TEXT("%s - %s"), *Config->AbilityName, *DisplayData.TargetingInfo);
	return DisplayData;
}

UTacGridSubsystem* UUnitAbilityInstance::GetGridSubsystem() const
{
	if (!Owner) return nullptr;
	UWorld* World = Owner->GetWorld();
	if (!World) return nullptr;
	return World->GetSubsystem<UTacGridSubsystem>();
}

UTacGridMovementService* UUnitAbilityInstance::GetMovementService() const
{
	UTacGridSubsystem* GridSys = GetGridSubsystem();
	return GridSys ? GridSys->GetGridMovementService() : nullptr;
}

UTacGridTargetingService* UUnitAbilityInstance::GetTargetingService() const
{
	UTacGridSubsystem* GridSys = GetGridSubsystem();
	return GridSys ? GridSys->GetGridTargetingService() : nullptr;
}

UTacCombatSubsystem* UUnitAbilityInstance::GetCombatSubsystem() const
{
	if (!Owner) return nullptr;
	UWorld* World = Owner->GetWorld();
	if (!World) return nullptr;
	return World->GetSubsystem<UTacCombatSubsystem>();
}

UTacAbilityExecutorService* UUnitAbilityInstance::GetExecutorService() const
{
	UTacCombatSubsystem* CombatSubsystem = GetCombatSubsystem();
	return CombatSubsystem ? CombatSubsystem->GetAbilityExecutorService() : nullptr;
}

UTacTurnSubsystem* UUnitAbilityInstance::GetTurnSubsystem() const
{
	if (!Owner) return nullptr;
	UWorld* World = Owner->GetWorld();
	if (!World) return nullptr;
	return World->GetSubsystem<UTacTurnSubsystem>();
}

bool UUnitAbilityInstance::IsOutsideFocus() const
{
	if (!Owner) return false;
	return (Owner->GetStats().Status.IsFocused() && !Owner->GetAbilityInventory()->IsFocusedOn(this));
}

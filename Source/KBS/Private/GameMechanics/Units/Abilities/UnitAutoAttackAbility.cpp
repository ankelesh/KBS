#include "GameMechanics/Units/Abilities/UnitAutoAttackAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/PresentationSubsystem.h"
#include "GameplayTypes/CombatTypes.h"
#include "Kismet/KismetMathLibrary.h"


ETargetReach UUnitAutoAttackAbility::GetTargeting() const
{
	if (Config->Targeting != ETargetReach::None)
	{
		return Config->Targeting;
	}
	else
	{
		UWeapon* Weapon = FDamageCalculation::SelectMaxReachWeapon(Owner);
		if (Weapon)
		{
			return Weapon->GetReach();
		}
	}
	return ETargetReach::None;
}
TMap<AUnit*, FPreviewHitResult> UUnitAutoAttackAbility::DamagePreview(AUnit* Source, const TArray<AUnit*>& Targets)
{
	TMap<AUnit*, FPreviewHitResult> Results;
	if (!Source)
	{
		return Results;
	}
	ETargetReach Reach = GetTargeting();
	for (AUnit* Target : Targets)
	{
		if (Target)
		{
			FPreviewHitResult Preview = FDamageCalculation::PreviewDamage(Source, Target, Reach);
			Results.Add(Target, Preview);
		}
	}
	return Results;
}
FAbilityResult UUnitAutoAttackAbility::ApplyAbilityEffect(AUnit* SourceUnit, FTacCoordinates TargetCell)
{
	if (!SourceUnit)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom, FText::FromString("No source unit"));
	}

	UWorld* World = SourceUnit->GetWorld();
	if (!World)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom, FText::FromString("No world"));
	}

	UTacCombatSubsystem* CombatSys = GetCombatSubsystem(SourceUnit);
	if (!CombatSys)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom, FText::FromString("Combat subsystem not available"));
	}

	UTacGridTargetingService* TargetingService = GetTargetingService(SourceUnit);
	if (!TargetingService)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom, FText::FromString("Targeting service not available"));
	}

	ETargetReach Reach = GetTargeting();
	TArray<AUnit*> TargetUnits = TargetingService->ResolveTargetsFromClick(SourceUnit, TargetCell, Reach);

	if (TargetUnits.Num() == 0)
	{
		return CreateFailureResult(EAbilityFailureReason::InvalidTarget, FText::FromString("No valid targets"));
	}

	// Play attack animation
	UWeapon* Weapon = FDamageCalculation::SelectMaxReachWeapon(SourceUnit);
	if (Weapon && TargetUnits.Num() > 0)
	{
		UWeaponDataAsset* WeaponConfig = Weapon->GetConfig();
		UAnimMontage* AttackMontage = WeaponConfig ? WeaponConfig->AttackMontage : nullptr;
		AUnit* FirstTarget = TargetUnits[0];
		if (FirstTarget)
		{
			const FVector SourceLoc = SourceUnit->GetActorLocation();
			const FVector TargetLoc = FirstTarget->GetActorLocation();
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(SourceLoc, TargetLoc);
			LookAtRotation.Yaw -= 90.0f;
			if (SourceUnit->VisualsComponent)
			{
				SourceUnit->VisualsComponent->RegisterRotationOperation();
				SourceUnit->VisualsComponent->RotateTowardTarget(LookAtRotation, 540.0f);
				if (AttackMontage)
				{
					SourceUnit->VisualsComponent->RegisterMontageOperation();
					SourceUnit->VisualsComponent->PlayAttackMontage(AttackMontage);
				}
			}
		}
	}

	// Process attack through combat subsystem
	FAbilityResult Result = CreateSuccessResult();

	// TODO: CombatSubsystem should orchestrate the full attack
	// Replace direct calls with:
	// FMutationResults MutationResults = CombatSys->ProcessUnitHit(SourceUnit, TargetUnits);
	// Result.UnitsAffected = MutationResults converted to affected units

	// Temporary: Direct processing until CombatSubsystem API is complete
	for (AUnit* Target : TargetUnits)
	{
		if (Target)
		{
			SourceUnit->OnUnitAttacks.Broadcast(SourceUnit, Target);
			Target->OnUnitAttacked.Broadcast(Target, SourceUnit);
			FCombatHitResult HitResult = FDamageCalculation::ProcessHit(SourceUnit, Target, Reach);
			if (HitResult.bHit)
			{
				Result.UnitsAffected.Add(Target);
				UE_LOG(LogTemp, Log, TEXT("  -> Target '%s' took %.1d damage (%.1d blocked)"),
					*Target->GetName(), HitResult.Damage, HitResult.DamageBlocked);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("  -> Target '%s' avoided the attack"),
					*Target->GetName());
			}
		}
	}
	return Result;
}


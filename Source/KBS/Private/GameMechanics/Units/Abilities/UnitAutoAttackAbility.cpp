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

	UTacGridTargetingService* TargetingService = GetTargetingService(Source);
	if (!TargetingService)
	{
		return Results;
	}

	for (AUnit* Target : Targets)
	{
		if (Target)
		{
			UWeapon* Weapon = TargetingService->SelectWeapon(Source, Target);
			if (Weapon)
			{
				FPreviewHitResult Preview = FDamageCalculation::PreviewDamage(Source, Weapon, Target);
				Results.Add(Target, Preview);
			}
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
	FResolvedTargets ResolvedTargets = TargetingService->ResolveTargetsFromClick(SourceUnit, TargetCell, Reach);

	// Check if we have a valid clicked target
	if (!ResolvedTargets.ClickedTarget)
	{
		return CreateFailureResult(EAbilityFailureReason::InvalidTarget, FText::FromString("No valid target at clicked cell"));
	}

	// Select weapon based on clicked target
	UWeapon* Weapon = TargetingService->SelectWeapon(SourceUnit, ResolvedTargets.ClickedTarget);
	if (!Weapon)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom, FText::FromString("No valid weapon for target"));
	}

	// Get all targets (clicked + secondary)
	TArray<AUnit*> AllTargets = ResolvedTargets.GetAllTargets();

	// Play attack animation
	UWeaponDataAsset* WeaponConfig = Weapon->GetConfig();
	UAnimMontage* AttackMontage = WeaponConfig ? WeaponConfig->AttackMontage : nullptr;
	const FVector SourceLoc = SourceUnit->GetActorLocation();
	const FVector TargetLoc = ResolvedTargets.ClickedTarget->GetActorLocation();
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

	// Process attack through combat subsystem
	TArray<FCombatHitResult> HitResults = CombatSys->ResolveAttack(SourceUnit, AllTargets, Weapon);

	FAbilityResult Result = CreateSuccessResult();
	for (const FCombatHitResult& HitResult : HitResults)
	{
		if (HitResult.bHit && HitResult.TargetUnit)
		{
			Result.UnitsAffected.Add(HitResult.TargetUnit);
		}
	}

	return Result;
}


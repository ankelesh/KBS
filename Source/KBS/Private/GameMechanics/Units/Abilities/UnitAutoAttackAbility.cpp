#include "GameMechanics/Units/Abilities/UnitAutoAttackAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/PresentationSubsystem.h"
#include "GameplayTypes/CombatTypes.h"


TMap<FTacCoordinates, FPreviewHitResult> UUnitAutoAttackAbility::DamagePreview(FTacCoordinates TargetCell) const
{
	TMap<FTacCoordinates, FPreviewHitResult> Results;
	if (!Owner) return Results;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return Results;

	ETargetReach Reach = GetTargeting();
	FResolvedTargets ResolvedTargets = TargetingService->ResolveTargetsFromClick(Owner, TargetCell, Reach);
	TArray<AUnit*> AllTargets = ResolvedTargets.GetAllTargets();

	for (AUnit* Target : AllTargets)
	{
		if (!Target) continue;

		UWeapon* Weapon = TargetingService->SelectWeapon(Owner, Target);
		if (!Weapon) continue;

		FPreviewHitResult Preview = FDamageCalculation::PreviewDamage(Owner, Weapon, Target);
		FTacCoordinates TargetCoords = Target->GetGridMetadata().Coords;
		Results.Add(TargetCoords, Preview);
	}

	return Results;
}

bool UUnitAutoAttackAbility::Execute(FTacCoordinates TargetCell)
{
	if (!Owner) return false;

	UTacCombatSubsystem* CombatSubsystem = GetCombatSubsystem();
	if (!CombatSubsystem) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	// Resolve targets
	ETargetReach Reach = GetTargeting();
	FResolvedTargets ResolvedTargets = TargetingService->ResolveTargetsFromClick(Owner, TargetCell, Reach);
	if (!ResolvedTargets.ClickedTarget) return false;

	// Select weapon for primary target
	UWeapon* Weapon = TargetingService->SelectWeapon(Owner, ResolvedTargets.ClickedTarget);
	if (!Weapon) return false;

	UPresentationSubsystem::FScopedBatch AttackBatch(
		UPresentationSubsystem::Get(Owner),
		FString::Printf(TEXT("Attack_%s"), *Owner->GetName())
	);

	// Play attack visuals
	if (Owner->VisualsComponent)
	{
		Owner->VisualsComponent->PlayAttackSequence(Owner, ResolvedTargets.ClickedTarget, Weapon);
	}

	// Execute attack through combat subsystem
	TArray<AUnit*> AllTargets = ResolvedTargets.GetAllTargets();
	TArray<FCombatHitResult> HitResults = CombatSubsystem->ResolveAttack(Owner, AllTargets, Weapon);

	bool bProcessingSucceeded = true;
	bool bAnyHitLanded = false;
	for (const FCombatHitResult& HitResult : HitResults)
	{
		if (!HitResult.bProcessingSucceeded)
		{
			bProcessingSucceeded = false;
		}
		if (HitResult.HitOutcome == EHitOutcome::Hit)
		{
			bAnyHitLanded = true;
		}
	}

	Owner->GetStats().Status.SetFocus();
	ConsumeCharge();

	return bProcessingSucceeded;
}

bool UUnitAutoAttackAbility::CanExecute(FTacCoordinates TargetCell) const
{
	if (!Owner || RemainingCharges <= 0 || IsOutsideFocus()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	return TargetingService->HasValidTargetAtCell(Owner, TargetCell, GetTargeting());
}

bool UUnitAutoAttackAbility::CanExecute() const
{
	if (!Owner || RemainingCharges <= 0 || IsOutsideFocus()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	return TargetingService->HasAnyValidTargets(Owner, GetTargeting());
}

ETargetReach UUnitAutoAttackAbility::GetTargeting() const
{
	if (Config->Targeting != ETargetReach::None)
	{
		return Config->Targeting;
	}
	else
	{
		if (const UWeapon* Weapon = FDamageCalculation::SelectMaxReachWeapon(Owner))
		{
			return Weapon->GetReach();
		}
	}
	return ETargetReach::None;
}



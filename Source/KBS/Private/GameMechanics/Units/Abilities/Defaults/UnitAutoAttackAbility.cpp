#include "GameMechanics/Units/Abilities/Defaults/UnitAutoAttackAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/PresentationSubsystem.h"
#include "GameplayTypes/CombatTypes.h"
#include "GameplayTypes/Tags/Tactical/AbilityTags.h"


TMap<FTacCoordinates, FPreviewHitResult> UUnitAutoAttackAbility::DamagePreview(FTacCoordinates TargetCell) const
{
	TMap<FTacCoordinates, FPreviewHitResult> Results;
	if (!Owner) return Results;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return Results;

	FTargetingDescriptor Targeting = GetTargeting();
	FResolvedTargets ResolvedTargets = TargetingService->ResolveTargetsFromClick(Owner, TargetCell, Targeting);
	TArray<AUnit*> AllTargets = ResolvedTargets.GetAllTargets();

	for (AUnit* Target : AllTargets)
	{
		if (!Target) continue;

		UCombatDescriptor* Weapon = FDamageCalculation::SelectDescriptorForTarget(Owner, Target, true);
		if (!Weapon) continue;

		FPreviewHitResult Preview = FDamageCalculation::PreviewDamage(Owner, Weapon, Target);
		FTacCoordinates TargetCoords = Target->GetGridMetadata().Coords;
		Results.Add(TargetCoords, Preview);
	}

	return Results;
}

FAbilityExecutionResult UUnitAutoAttackAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	UTacCombatSubsystem* CombatSubsystem = GetCombatSubsystem();
	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	check(CombatSubsystem);

	// Resolve targets
	FTargetingDescriptor Targeting = GetTargeting();
	FResolvedTargets ResolvedTargets = TargetingService->ResolveTargetsFromClick(Owner, TargetCell, Targeting);
	if (!ResolvedTargets.ClickedTarget) return FAbilityExecutionResult::MakeFail();

	// Select weapon for primary target
	UCombatDescriptor* Weapon = FDamageCalculation::SelectDescriptorForTarget(Owner, ResolvedTargets.ClickedTarget, true);
	check(Weapon);

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

	ConsumeCharge();
	SetCompletionTag();

	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UUnitAutoAttackAbility::CanExecute(FTacCoordinates TargetCell) const
{
	if (!Owner || RemainingCharges <= 0 || !OwnerCanAct() || !CanActByContext()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	return TargetingService->HasValidTargetAtCell(Owner, TargetCell, GetTargeting());
}

bool UUnitAutoAttackAbility::CanExecute() const
{
	if (!Owner || RemainingCharges <= 0 || !OwnerCanAct() || !CanActByContext()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	return TargetingService->HasAnyValidTargets(Owner, GetTargeting());
}

EAbilityTurnReleasePolicy UUnitAutoAttackAbility::DecideTurnRelease() const
{
	if (RemainingCharges > 0)
		return EAbilityTurnReleasePolicy::Locked;
	return Super::DecideTurnRelease();
}

FTargetingDescriptor UUnitAutoAttackAbility::GetTargeting() const
{
	if (Config->Targeting != ETargetReach::None)
	{
		return FTargetingDescriptor::FromReach(Config->Targeting);
	}
	if (UCombatDescriptor* Weapon = FDamageCalculation::SelectMaxReachDescriptor(Owner, true))
	{
		return Weapon->GetTargeting();
	}
	return FTargetingDescriptor{};
}

FGameplayTagContainer UUnitAutoAttackAbility::BuildTags() const
{
	FGameplayTagContainer Tags = Super::BuildTags();
	if (UCombatDescriptor* Weapon = FDamageCalculation::SelectMaxReachDescriptor(Owner, true))
	{
		const EMagnitudePolicy Policy = Weapon->GetMagnitudePolicy();
		// AutoAttack alters Damage policy to its own class tag; other policies map generically
		if (Policy == EMagnitudePolicy::Damage)
			Tags.AddTag(TAG_ABILITY_AUTOATTACK);
		else if (FGameplayTag PolicyTag = AbilityTagUtils::TagFromPolicy(Policy); PolicyTag.IsValid())
			Tags.AddTag(PolicyTag);
	}
	return Tags;
}



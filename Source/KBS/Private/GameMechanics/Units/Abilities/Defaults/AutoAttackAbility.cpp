#include "GameMechanics/Units/Abilities/Defaults/AutoAttackAbility.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Combat/Weapon.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameplayTypes/CombatTypes.h"
#include "GameplayTypes/Tags/Tactical/AbilityTags.h"


TMap<FTacCoordinates, FPreviewHitResult> UAutoAttackAbility::DamagePreview(FTacCoordinates TargetCell) const
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

		UWeapon* Weapon = FDamageCalculation::SelectWeaponForTarget(Owner, Target, true);
		if (!Weapon) continue;

		FPreviewHitResult Preview = FDamageCalculation::PreviewDamage(Owner, Weapon->GetDescriptor(), Target);
		FTacCoordinates TargetCoords = Target->GetGridMetadata().Coords;
		Results.Add(TargetCoords, Preview);
	}

	return Results;
}

FAbilityExecutionResult UAutoAttackAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	UTacCombatSubsystem* CombatSubsystem = GetCombatSubsystem();
	UTacGridTargetingService* TargetingService = GetTargetingService();
	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(TargetingService);
	check(CombatSubsystem);
	check(LogSubsystem);

	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::Ability, ETacLogEventOrigin::Initiated,
	                                        Owner->GetUnitID(), FGuid());

	// Resolve targets
	FTargetingDescriptor Targeting = GetTargeting();
	FResolvedTargets ResolvedTargets = TargetingService->ResolveTargetsFromClick(Owner, TargetCell, Targeting);
	if (!ResolvedTargets.ClickedTarget) return FAbilityExecutionResult::MakeFail();

	// Select weapon for primary target
	UWeapon* Weapon = FDamageCalculation::SelectWeaponForTarget(Owner, ResolvedTargets.ClickedTarget, true);
	check(Weapon);

	// Execute attack through combat subsystem
	TArray<AUnit*> AllTargets = ResolvedTargets.GetAllTargets();
	TArray<FCombatHitResult> HitResults = CombatSubsystem->ResolveAttack(Owner, AllTargets, Weapon->GetDescriptor());

	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Command           = TargetCell;
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogCombatStep>(
		FTacLogCombatStep::Make(Owner->GetUnitID(), Owner->GetGridMetadata().Coords,
		                        ResolvedTargets.ClickedTarget->GetUnitID(), HitResults, Weapon->GetAnimTag())));
	FTacLogEffectSpawnStep::AppendFromHits(Payload.Steps, HitResults);
	FTacLogStatusChangeStep::AppendDefendingClearedFromHits(Payload.Steps, HitResults);
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));

	ConsumeCharge();
	SetCompletionTag();

	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UAutoAttackAbility::CanExecute(FTacCoordinates TargetCell) const
{
	if (!Owner || RemainingCharges <= 0 || !OwnerCanAct() || !CanActByContext()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	return TargetingService->HasValidTargetAtCell(Owner, TargetCell, GetTargeting());
}

bool UAutoAttackAbility::CanExecute() const
{
	if (!Owner || RemainingCharges <= 0 || !OwnerCanAct() || !CanActByContext()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	return TargetingService->HasAnyValidTargets(Owner, GetTargeting());
}

EAbilityTurnReleasePolicy UAutoAttackAbility::DecideTurnRelease() const
{
	if (RemainingCharges > 0)
		return EAbilityTurnReleasePolicy::Locked;
	return Super::DecideTurnRelease();
}

FTargetingDescriptor UAutoAttackAbility::GetTargeting() const
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

FGameplayTagContainer UAutoAttackAbility::BuildTags() const
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
		AbilityTagUtils::AddEffectTags(Tags, Weapon->GetEffects());
	}
	return Tags;
}



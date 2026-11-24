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

		UWeapon* Weapon = FDamageCalculation::SelectWeaponForTarget(Owner, Target, true, GetExcludedWeapon());
		if (!Weapon) continue;

		FPreviewHitResult Preview = FDamageCalculation::PreviewDamage(Owner, ResolveAttackDescriptor(Weapon->GetDescriptor()), Target);
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
	UWeapon* Weapon = FDamageCalculation::SelectWeaponForTarget(Owner, ResolvedTargets.ClickedTarget, true, GetExcludedWeapon());
	check(Weapon);

	// Execute attack through combat subsystem
	TArray<AUnit*> AllTargets = ResolvedTargets.GetAllTargets();
	UCombatDescriptor* AttackDescriptor = ResolveAttackDescriptor(Weapon->GetDescriptor());
	TArray<FCombatHitResult> HitResults = CombatSubsystem->ResolveAttack(Owner, AllTargets, AttackDescriptor);

	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Command           = TargetCell;
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogCombatStep>(
		FTacLogCombatStep::Make(Owner->GetUnitID(), Owner->GetGridMetadata().Coords,
		                        ResolvedTargets.ClickedTarget->GetUnitID(), HitResults, Weapon->GetAnimTag(), Weapon->GetDeliveryManifestPath())));
	FTacLogEffectSpawnStep::AppendFromHits(Payload.Steps, HitResults);
	FTacLogStatusChangeStep::AppendDefendingClearedFromHits(Payload.Steps, HitResults);

	const FUnitChargeCost PaidCost = Owner->PayWeaponCost(*Weapon);
	AppendChargeStep(Payload.Steps, PaidCost.Tag, -PaidCost.Amount);
	OnWeaponUsed(*Weapon);
	OnAttackResolved(AttackDescriptor, Payload.Steps);
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));

	ConsumeUse();
	SetCompletionTag();

	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UAutoAttackAbility::CanExecute(FTacCoordinates TargetCell) const
{
	if (!Owner || RemainingUses <= 0 || !OwnerCanAct() || !CanActByContext() || !HasUsableWeapon()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	return TargetingService->HasValidTargetAtCell(Owner, TargetCell, GetTargeting());
}

bool UAutoAttackAbility::CanExecute() const
{
	if (!Owner || RemainingUses <= 0 || !OwnerCanAct() || !CanActByContext() || !HasUsableWeapon()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	return TargetingService->HasAnyValidTargets(Owner, GetTargeting());
}

EAbilityTurnReleasePolicy UAutoAttackAbility::DecideTurnRelease() const
{
	// Lock only when a follow-up attack is actually possible, otherwise the turn would be held by an unusable ability
	if (RemainingUses > 0 && HasUsableWeapon())
		return EAbilityTurnReleasePolicy::Locked;
	return Super::DecideTurnRelease();
}

bool UAutoAttackAbility::HasUsableWeapon() const
{
	return FDamageCalculation::SelectMaxReachDescriptor(Owner, true, GetExcludedWeapon()) != nullptr;
}

FTargetingDescriptor UAutoAttackAbility::GetTargeting() const
{
	if (Config->Targeting != ETargetReach::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoAttack '%s': Config Targeting is ignored, targeting comes from weapons"),
		       *Config->AbilityName);
	}
	if (UCombatDescriptor* Weapon = FDamageCalculation::SelectMaxReachDescriptor(Owner, true, GetExcludedWeapon()))
	{
		return ResolveAttackDescriptor(Weapon)->GetTargeting();
	}
	return FTargetingDescriptor{};
}

FGameplayTagContainer UAutoAttackAbility::BuildTags() const
{
	FGameplayTagContainer Tags = Super::BuildTags();
	if (UCombatDescriptor* WeaponDescriptor = FDamageCalculation::SelectMaxReachDescriptor(Owner, true))
	{
		TArray<UCombatDescriptor*> Descriptors;
		GatherTagDescriptors(WeaponDescriptor, Descriptors);
		for (UCombatDescriptor* Descriptor : Descriptors)
		{
			const EMagnitudePolicy Policy = Descriptor->GetMagnitudePolicy();
			// AutoAttack alters Damage policy to its own class tag; other policies map generically
			if (Policy == EMagnitudePolicy::Damage)
				Tags.AddTag(TAG_ABILITY_AUTOATTACK);
			else if (FGameplayTag PolicyTag = AbilityTagUtils::TagFromPolicy(Policy); PolicyTag.IsValid())
				Tags.AddTag(PolicyTag);
			AbilityTagUtils::AddEffectTags(Tags, Descriptor->GetEffects());
		}
	}
	return Tags;
}



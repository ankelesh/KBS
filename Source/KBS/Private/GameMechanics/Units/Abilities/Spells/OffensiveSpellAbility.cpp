#include "GameMechanics/Units/Abilities/Spells/OffensiveSpellAbility.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Units/Abilities/Spells/OffensiveSpellAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameplayTypes/CombatTypes.h"
#include "GameplayTypes/Tags/Tactical/AbilityTags.h"


void UOffensiveSpellAbility::InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner)
{
	Super::InitializeFromDefinition(InDefinition, InOwner);

	UOffensiveSpellAbilityDefinition* SpellConfig = Cast<UOffensiveSpellAbilityDefinition>(Config);
	checkf(SpellConfig, TEXT("UOffensiveSpellAbility requires a USpellAbilityDefinition asset"));

	EmbeddedDescriptor = NewObject<UCombatDescriptor>(this);
	EmbeddedDescriptor->Initialize(this, SpellConfig->EmbeddedDescriptor);
}

void UOffensiveSpellAbility::ScaleEmbeddedDescriptor() const
{
	UOffensiveSpellAbilityDefinition* SpellConfig = Cast<UOffensiveSpellAbilityDefinition>(Config);
	if (SpellConfig->bIgnoreScaling) return;

	UCombatDescriptor* SpellDescriptor = FDamageCalculation::SelectSpellDescriptor(Owner);
	checkf(SpellDescriptor, TEXT("UOffensiveSpellAbility: unit has no Spells-designated descriptor to drive scaling"));

	FDamageCalculation::ApplySpellScaling(EmbeddedDescriptor, SpellDescriptor, SpellConfig->DamageMultiplier, SpellConfig->FlatDamageBonus);
}

TMap<FTacCoordinates, FPreviewHitResult> UOffensiveSpellAbility::DamagePreview(FTacCoordinates TargetCell) const
{
	TMap<FTacCoordinates, FPreviewHitResult> Results;
	if (!Owner) return Results;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return Results;

	ScaleEmbeddedDescriptor();

	FResolvedTargets ResolvedTargets = TargetingService->ResolveTargetsFromClick(Owner, TargetCell, GetTargeting(), &EmbeddedDescriptor->GetStats().AreaShape);
	for (AUnit* Target : ResolvedTargets.GetAllTargets())
	{
		if (!Target) continue;
		FPreviewHitResult Preview = FDamageCalculation::PreviewDamage(Owner, EmbeddedDescriptor, Target);
		Results.Add(Target->GetGridMetadata().Coords, Preview);
	}
	return Results;
}

FAbilityExecutionResult UOffensiveSpellAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	UTacCombatSubsystem* CombatSubsystem = GetCombatSubsystem();
	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(CombatSubsystem);
	check(LogSubsystem);
	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	FResolvedTargets ResolvedTargets = TargetingService->ResolveTargetsFromClick(Owner, TargetCell, GetTargeting(), &EmbeddedDescriptor->GetStats().AreaShape);
	if (!ResolvedTargets.ClickedTarget) return FAbilityExecutionResult::MakeFail();

	ScaleEmbeddedDescriptor();

	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::Ability, ETacLogEventOrigin::Initiated,
	                                        Owner->GetUnitID(), FGuid());

	TArray<AUnit*> AllTargets = ResolvedTargets.GetAllTargets();
	TArray<FCombatHitResult> HitResults = CombatSubsystem->ResolveAttack(Owner, AllTargets, EmbeddedDescriptor);

	const UOffensiveSpellAbilityDefinition* SpellDef = CastChecked<UOffensiveSpellAbilityDefinition>(Config);

	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Command           = TargetCell;
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogCombatStep>(
		FTacLogCombatStep::Make(Owner->GetUnitID(), Owner->GetGridMetadata().Coords,
		                        ResolvedTargets.ClickedTarget->GetUnitID(), HitResults, SpellDef->AnimTag)));
	FTacLogEffectSpawnStep::AppendFromHits(Payload.Steps, HitResults);
	FTacLogStatusChangeStep::AppendDefendingClearedFromHits(Payload.Steps, HitResults);
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));

	SetCompletionTag();
	ConsumeCharge();

	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UOffensiveSpellAbility::CanExecute(FTacCoordinates TargetCell) const
{
	if (!Owner || RemainingCharges <= 0 || !OwnerCanAct() || !CanActByContext()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	return TargetingService->HasValidTargetAtCell(Owner, TargetCell, GetTargeting());
}

bool UOffensiveSpellAbility::CanExecute() const
{
	if (!Owner || RemainingCharges <= 0 || !OwnerCanAct() || !CanActByContext()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	return TargetingService->HasAnyValidTargets(Owner, GetTargeting());
}

FTargetingDescriptor UOffensiveSpellAbility::GetTargeting() const
{
	if (Config->Targeting != ETargetReach::None)
	{
		return FTargetingDescriptor::FromReach(Config->Targeting);
	}
	return EmbeddedDescriptor->GetTargeting();
}

FGameplayTagContainer UOffensiveSpellAbility::BuildTags() const
{
	FGameplayTagContainer Tags = Super::BuildTags();
	Tags.AddTag(TAG_ABILITY_SPELL);
	if (EmbeddedDescriptor)
	{
		if (FGameplayTag PolicyTag = AbilityTagUtils::TagFromPolicy(EmbeddedDescriptor->GetMagnitudePolicy()); PolicyTag.IsValid())
			Tags.AddTag(PolicyTag);
		AbilityTagUtils::AddEffectTags(Tags, EmbeddedDescriptor->GetEffects());
	}
	return Tags;
}

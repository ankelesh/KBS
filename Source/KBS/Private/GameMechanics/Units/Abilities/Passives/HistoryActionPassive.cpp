#include "GameMechanics/Units/Abilities/Passives/HistoryActionPassive.h"
#include "GameMechanics/Units/Abilities/Passives/HistoryActionPassiveDefinition.h"
#include "GameMechanics/Units/Components/UnitActionHistoryComponent.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"

UHistoryActionModifyStatsPassiveDefinition::UHistoryActionModifyStatsPassiveDefinition()
{
	AbilityClass = UHistoryActionModifyStatsPassive::StaticClass();
}

UHistoryActionApplyEffectPassiveDefinition::UHistoryActionApplyEffectPassiveDefinition()
{
	AbilityClass = UHistoryActionApplyEffectPassive::StaticClass();
}

void UHistoryActionPassive::Subscribe()
{
	UUnitActionHistoryComponent& History = Owner->GetActionHistory();
	const FGameplayTag& TriggerTag = GetTriggerConfig().TriggerTag;
	// Designer data drift: report and stay inert instead of crashing the battle.
	if (!History.IsTrackedAction(TriggerTag))
	{
		UE_LOG(LogKBSCombat, Error, TEXT("%s on %s: trigger tag '%s' is not a tracked action of the unit's history; passive will never fire"),
		       *Config->AbilityName, *Owner->GetLogName(), *TriggerTag.ToString());
		return;
	}
	History.OnActionRecorded.AddDynamic(this, &UHistoryActionPassive::HandleActionRecorded);
}

void UHistoryActionPassive::Unsubscribe()
{
	Owner->GetActionHistory().OnActionRecorded.RemoveDynamic(this, &UHistoryActionPassive::HandleActionRecorded);
}

const UHistoryActionPassiveDefinition& UHistoryActionPassive::GetTriggerConfig() const
{
	const UHistoryActionPassiveDefinition* TriggerConfig = Cast<UHistoryActionPassiveDefinition>(Config);
	checkf(TriggerConfig, TEXT("UHistoryActionPassive requires an UHistoryActionPassiveDefinition"));
	return *TriggerConfig;
}

void UHistoryActionPassive::HandleActionRecorded(const FGameplayTag& ActionTag, bool bChangedFromPrevious)
{
	const UHistoryActionPassiveDefinition& TriggerConfig = GetTriggerConfig();
	if (ActionTag != TriggerConfig.TriggerTag)
		return;
	if (TriggerConfig.bRequireChange && !bChangedFromPrevious)
		return;
	OnTriggered();
}

void UHistoryActionPassive::LogReaction(TArray<TInstancedStruct<FTacLogStepBase>>&& Steps) const
{
	if (Steps.IsEmpty())
		return;

	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(LogSubsystem);
	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Steps             = MoveTemp(Steps);
	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::Ability, ETacLogEventOrigin::Reaction,
	                                        Owner->GetUnitID(), LogSubsystem->FindLatestEvent());
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));
}

void UHistoryActionModifyStatsPassive::OnTriggered()
{
	const FUnitInstantChange& Change = CastChecked<UHistoryActionModifyStatsPassiveDefinition>(Config)->Change;
	const int32 HealApplied = Owner->ApplyInstantChange(Change);

	TArray<TInstancedStruct<FTacLogStepBase>> Steps;
	Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogInstantChangeStep>(
		FTacLogInstantChangeStep::Make(Owner->GetUnitID(), HealApplied, Owner->GetStats().Health.GetCurrent(),
		                               Change.WardsToGrant)));
	LogReaction(MoveTemp(Steps));
}

void UHistoryActionApplyEffectPassive::OnTriggered()
{
	TArray<TInstancedStruct<FTacLogStepBase>> Steps;
	ApplyConfiguredEffects(Owner, CastChecked<UHistoryActionApplyEffectPassiveDefinition>(Config)->Effects, Steps);
	LogReaction(MoveTemp(Steps));
}

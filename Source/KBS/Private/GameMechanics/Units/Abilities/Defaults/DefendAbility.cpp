#include "GameMechanics/Units/Abilities/Defaults/DefendAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameplayTypes/Tags/Tactical/AbilityTags.h"


FAbilityExecutionResult UDefendAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(LogSubsystem);

	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::Ability, ETacLogEventOrigin::Initiated,
	                                        Owner->GetUnitID(), FGuid());

	Owner->GetStats().Status.SetDefending();

	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Command           = TargetCell;
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogStatusChangeStep>(
		FTacLogStatusChangeStep::Make(Owner->GetUnitID(), EUnitStatus::Defending, true)));
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));

	ConsumeCharge();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UDefendAbility::CanExecute(FTacCoordinates TargetCell) const
{
	return Owner && RemainingCharges > 0 && OwnerCanAct() && CanActByContext();
}

bool UDefendAbility::CanExecute() const
{
	return Owner && RemainingCharges > 0 && OwnerCanAct() && CanActByContext();
}

FGameplayTagContainer UDefendAbility::BuildTags() const
{
	FGameplayTagContainer Tags = Super::BuildTags();
	Tags.AddTag(TAG_ABILITY_DEFEND);
	return Tags;
}


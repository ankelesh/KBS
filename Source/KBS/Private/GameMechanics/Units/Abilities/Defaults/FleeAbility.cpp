#include "GameMechanics/Units/Abilities/Defaults/FleeAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/Tags/Tactical/AbilityTags.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"


FAbilityExecutionResult UFleeAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(LogSubsystem);
	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::Ability, ETacLogEventOrigin::Initiated,
	                                        Owner->GetUnitID(), FGuid());

	Owner->GetStats().Status.SetFleeing();
	Owner->OnUnitTurnStart.AddDynamic(this, &UFleeAbility::HandleTurnStarted);

	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Command           = TargetCell;
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogFleeStep>(
		FTacLogFleeStep::Make(Owner->GetUnitID(), Owner->GetGridMetadata().Coords, Owner->GetTeamSide())));
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));

	ConsumeCharge();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UFleeAbility::CanExecute(FTacCoordinates TargetCell) const
{
	return CanExecute();
}

bool UFleeAbility::CanExecute() const
{
	check(Owner);
	return OwnerCanAct() && CanActByContext() && RemainingCharges > 0;
}

void UFleeAbility::HandleTurnStarted(AUnit* Unit)
{
	const bool bWillFlee = Owner->GetStats().Status.IsFleeing()
		&& Owner->GetStats().Status.CanAct()
		&& Owner->GetStats().Status.CanMove();

	const FTacCoordinates LastFieldCoords = Owner->GetGridMetadata().Coords;

	if (bWillFlee)
		GetGridSubsystem()->PlaceUnitOffField(Owner);

	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(LogSubsystem);
	FUnitMoveOffFieldPayload Payload;
	Payload.UnitId          = Owner->GetUnitID();
	Payload.LastFieldCoords = LastFieldCoords;
	Payload.TeamSide        = Owner->GetTeamSide();
	Payload.bFled           = bWillFlee;
	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::UnitExitField, ETacLogEventOrigin::Triggered,
	                                        Owner->GetUnitID(), FGuid());
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FUnitMoveOffFieldPayload>(MoveTemp(Payload)));

	Owner->OnUnitTurnStart.RemoveDynamic(this, &UFleeAbility::HandleTurnStarted);
}

FGameplayTagContainer UFleeAbility::BuildTags() const
{
	FGameplayTagContainer Tags = Super::BuildTags();
	Tags.AddTag(TAG_ABILITY_FLEE);
	return Tags;
}

#include "GameMechanics/Units/Abilities/Defaults/FleeAbility.h"
#include "GameMechanics/Units/Abilities/Defaults/FleeAbilityDefinition.h"
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

	if (GetFleeConfig()->DepartureMode == EDepartureMode::Flee)
		Owner->GetStats().Status.SetFleeing();
	ScheduleDeparture();

	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Command           = TargetCell;
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogFleeStep>(
		FTacLogFleeStep::Make(Owner->GetUnitID(), Owner->GetGridMetadata().Coords, Owner->GetTeamSide())));
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));

	ConsumeUse();
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
	return OwnerCanAct() && CanActByContext() && RemainingUses > 0;
}

const UFleeAbilityDefinition* UFleeAbility::GetFleeConfig() const
{
	return CastChecked<UFleeAbilityDefinition>(Config);
}

void UFleeAbility::ScheduleDeparture()
{
	switch (GetFleeConfig()->Timing)
	{
	case EFleeTiming::NextTurnStart:
		Owner->OnUnitTurnStart.AddUniqueDynamic(this, &UFleeAbility::HandleTurnStarted);
		break;
	case EFleeTiming::ThisTurnEnd:
	case EFleeTiming::Instant:
		Owner->OnUnitTurnEnd.AddUniqueDynamic(this, &UFleeAbility::HandleTurnEnded);
		break;
	}
}

void UFleeAbility::HandleTurnStarted(AUnit* Unit)
{
	Owner->OnUnitTurnStart.RemoveDynamic(this, &UFleeAbility::HandleTurnStarted);
	Depart();
}

void UFleeAbility::HandleTurnEnded(AUnit* Unit)
{
	Owner->OnUnitTurnEnd.RemoveDynamic(this, &UFleeAbility::HandleTurnEnded);
	Depart();
}

void UFleeAbility::Depart()
{
	const bool bFleeMode = GetFleeConfig()->DepartureMode == EDepartureMode::Flee;
	const FUnitStatusContainer& Status = Owner->GetStats().Status;
	const bool bWillLeave = Status.CanAct() && Status.CanMove() && (!bFleeMode || Status.IsFleeing());

	const FTacCoordinates LastFieldCoords = Owner->GetGridMetadata().Coords;

	if (bWillLeave)
		GetGridSubsystem()->PlaceUnitOffField(Owner);

	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(LogSubsystem);
	FUnitMoveOffFieldPayload Payload;
	Payload.UnitId          = Owner->GetUnitID();
	Payload.LastFieldCoords = LastFieldCoords;
	Payload.TeamSide        = Owner->GetTeamSide();
	Payload.bFled           = bWillLeave && bFleeMode;
	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::UnitExitField, ETacLogEventOrigin::Triggered,
	                                        Owner->GetUnitID(), FGuid());
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FUnitMoveOffFieldPayload>(MoveTemp(Payload)));
}

FGameplayTagContainer UFleeAbility::BuildTags() const
{
	FGameplayTagContainer Tags = Super::BuildTags();
	Tags.AddTag(TAG_ABILITY_FLEE);
	return Tags;
}

#include "GameMechanics/Units/Abilities/Defaults/WaitAbility.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "GameplayTypes/Tags/Tactical/AbilityTags.h"


FAbilityExecutionResult UWaitAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	UTacTurnSubsystem* TurnSubsystem = GetTurnSubsystem();
	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(TurnSubsystem);
	check(LogSubsystem);

	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::Ability, ETacLogEventOrigin::Initiated,
	                                        Owner->GetUnitID(), FGuid());

	TurnSubsystem->Wait();

	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Command           = TargetCell;
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogWaitStep>(
		FTacLogWaitStep::Make(Owner->GetUnitID())));
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));

	UE_LOG(LogTemp, Log, TEXT("%s uses Wait - reinserted into queue with modified initiative"), *Owner->GetName());
	ConsumeCharge();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UWaitAbility::CanExecute(FTacCoordinates TargetCell) const
{
	check(Owner);
	return RemainingCharges > 0 && OwnerCanAct() && CanActByContext();
}

bool UWaitAbility::CanExecute() const
{
	return Owner && RemainingCharges > 0 && OwnerCanAct() && CanActByContext();
}

void UWaitAbility::Subscribe()
{
	UTacTurnSubsystem* TurnSubsystem = GetTurnSubsystem();
	check(TurnSubsystem);
	TurnSubsystem->OnRoundEnd.AddDynamic(this, &UWaitAbility::HandleRoundEnd);
}

void UWaitAbility::Unsubscribe()
{
	UTacTurnSubsystem* TurnSubsystem = GetTurnSubsystem();
	check(TurnSubsystem);
	TurnSubsystem->OnRoundEnd.RemoveDynamic(this, &UWaitAbility::HandleRoundEnd);
}

void UWaitAbility::HandleRoundEnd(int32)
{
	RestoreCharges();
}

FGameplayTagContainer UWaitAbility::BuildTags() const
{
	FGameplayTagContainer Tags = Super::BuildTags();
	Tags.AddTag(TAG_ABILITY_WAIT);
	return Tags;
}


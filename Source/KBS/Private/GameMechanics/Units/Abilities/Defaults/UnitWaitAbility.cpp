#include "GameMechanics/Units/Abilities/Defaults/UnitWaitAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"


FAbilityExecutionResult UUnitWaitAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	UTacTurnSubsystem* TurnSubsystem = GetTurnSubsystem();
	check(TurnSubsystem);
	
	TurnSubsystem->Wait();
	
	UE_LOG(LogTemp, Log, TEXT("%s uses Wait - reinserted into queue with modified initiative"), *Owner->GetName());
	ConsumeCharge();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UUnitWaitAbility::CanExecute(FTacCoordinates TargetCell) const
{
	check(Owner);
	return RemainingCharges > 0 && OwnerCanAct() && CanActByContext();
}

bool UUnitWaitAbility::CanExecute() const
{
	return Owner && RemainingCharges > 0 && OwnerCanAct() && CanActByContext();
}

void UUnitWaitAbility::Subscribe()
{
	UTacTurnSubsystem* TurnSubsystem = GetTurnSubsystem();
	check(TurnSubsystem);
	TurnSubsystem->OnRoundEnd.AddDynamic(this, &UUnitWaitAbility::HandleRoundEnd);
}

void UUnitWaitAbility::Unsubscribe()
{
	UTacTurnSubsystem* TurnSubsystem = GetTurnSubsystem();
	check(TurnSubsystem);
	TurnSubsystem->OnRoundEnd.RemoveDynamic(this, &UUnitWaitAbility::HandleRoundEnd);
}

void UUnitWaitAbility::HandleRoundEnd(int32)
{
	RestoreCharges();
}


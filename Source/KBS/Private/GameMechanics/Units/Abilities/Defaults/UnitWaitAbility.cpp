#include "GameMechanics/Units/Abilities/Defaults/UnitWaitAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"


bool UUnitWaitAbility::Execute(FTacCoordinates TargetCell)
{
	if (!Owner || IsOutsideFocus()) return false;

	UTacTurnSubsystem* TurnSubsystem = GetTurnSubsystem();
	if (!TurnSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UnitWaitAbility: Failed to get TurnSubsystem"));
		return false;
	}

	// Invoke Wait on turn order
	TurnSubsystem->Wait();

	UE_LOG(LogTemp, Log, TEXT("%s uses Wait - reinserted into queue with modified initiative"), *Owner->GetName());

	// Set focused status to end turn
	Owner->GetStats().Status.SetFocus();
	ConsumeCharge();

	return true;
}

bool UUnitWaitAbility::CanExecute(FTacCoordinates TargetCell) const
{
	return Owner && RemainingCharges > 0 && !IsOutsideFocus();
}

bool UUnitWaitAbility::CanExecute() const
{
	return Owner && RemainingCharges > 0 && !IsOutsideFocus();
}

void UUnitWaitAbility::Subscribe()
{
	if (UTacTurnSubsystem* TurnSubsystem = GetTurnSubsystem())
	{
		TurnSubsystem->OnRoundEnd.AddDynamic(this, &UUnitWaitAbility::HandleRoundEnd);
	}
}

void UUnitWaitAbility::Unsubscribe()
{
	UTacTurnSubsystem* TurnSubsystem = GetTurnSubsystem();
	if (TurnSubsystem)
	{
		TurnSubsystem->OnRoundEnd.RemoveDynamic(this, &UUnitWaitAbility::HandleRoundEnd);
	}
}

void UUnitWaitAbility::HandleRoundEnd(int32)
{
	RestoreCharges();
}


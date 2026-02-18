#include "GameMechanics/Tactical/Grid/Subsystems/TacSubsystemControl.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"

void UTacSubsystemControl::NotifyGridReady()
{
	bGridReadyForStart = true;
	GridReadyForStart.Broadcast();
	CheckReady();
}

void UTacSubsystemControl::NotifyTurnReady()
{
	bTurnReadyForStart = true;
	TurnReadyForStart.Broadcast();
	CheckReady();
}

bool UTacSubsystemControl::CheckReady()
{
	if (IsReadyForStart())
	{
		ReadyForStart.Broadcast();
		StartBattle();
		return true;
	}
	return false;
}

void UTacSubsystemControl::StartBattle()
{
	UTacTurnSubsystem* TurnSubsystem = GetWorld()->GetSubsystem<UTacTurnSubsystem>();
	checkf(TurnSubsystem, TEXT("TacSubsystemControl: TacTurnSubsystem is null"));
	TurnSubsystem->StartBattle();
}

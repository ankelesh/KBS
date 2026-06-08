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
	if (bBattleStarted || !IsReadyForStart()) return false;
	bBattleStarted = true;
	ReadyForStart.Broadcast();
	StartBattle();
	return true;
}

void UTacSubsystemControl::StartBattle()
{
	UTacTurnSubsystem* TurnSubsystem = GetWorld()->GetSubsystem<UTacTurnSubsystem>();
	checkf(TurnSubsystem, TEXT("TacSubsystemControl: TacTurnSubsystem is null"));
	TurnSubsystem->StartBattle();
}

#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/TacTurnOrder.h"

void UTacTurnSubsystem::ReloadTurnOrder()
{
	UTacGridSubsystem* Grid = GetWorld()->GetSubsystem<UTacGridSubsystem>();
	if (!Grid) return;
	TurnOrder->Repopulate(Grid->GetActiveUnits(), Grid->GetAttackerTeam());
}

void UTacTurnSubsystem::BroadcastRoundStart()
{
	OnRoundStart.Broadcast(CurrentRound);
}

void UTacTurnSubsystem::BroadcastRoundEnd()
{
	OnRoundEnd.Broadcast(CurrentRound);
}

void UTacTurnSubsystem::BroadcastTurnStart()
{
	OnTurnStart.Broadcast(TurnOrder->GetCurrentUnit());
}

void UTacTurnSubsystem::BroadcastTurnEnd()
{
	OnTurnEnd.Broadcast(TurnOrder->GetCurrentUnit());
}

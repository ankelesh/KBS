#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/TacTurnState.h"

#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"

void FTacTurnState::Enter()
{
}

void FTacTurnState::Exit()
{
}


void FTacTurnState::BroadcastRoundStart()
{
	ParentTurnSubsystem->BroadcastRoundStart();
}

void FTacTurnState::BroadcastRoundEnd()
{
	ParentTurnSubsystem->BroadcastRoundEnd();
}

void FTacTurnState::BroadcastTurnEnd()
{
	ParentTurnSubsystem->BroadcastTurnEnd();
}

void FTacTurnState::BroadcastTurnStart()
{
	ParentTurnSubsystem->BroadcastTurnStart();
}

FTacTurnOrder* FTacTurnState::GetTurnOrder()
{
	if (!ParentTurnSubsystem) return nullptr;
	return ParentTurnSubsystem->TurnOrder.Get();
}

void FTacTurnState::ReloadTurnOrder()
{
	if (!ParentTurnSubsystem) return;
	ParentTurnSubsystem->ReloadTurnOrder();
}

#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/RoundStartState.h"

void FRoundStartState::Enter()
{
	ReloadTurnOrder();
}

void FRoundStartState::Exit()
{
	BroadcastRoundStart();
}

ETurnState FRoundStartState::NextState()
{
	return ETurnState::ETurnStartState;
}

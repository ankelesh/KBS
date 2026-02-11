#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/RoundEndState.h"

void FRoundEndState::Enter()
{
	FTacTurnState::Enter();
	BroadcastRoundEnd();
	ReloadTurnOrder();
}

ETurnState FRoundEndState::NextState()
{
	return ETurnState::ERoundStartState;
}

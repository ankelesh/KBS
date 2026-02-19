#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/RoundEndState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"

void FRoundEndState::Enter()
{
	FTacTurnState::Enter();
	UE_LOG(LogKBSTurn, Log, TEXT("=== Round %d end ==="), ParentTurnSubsystem->GetCurrentRound());
	BroadcastRoundEnd();
	ReloadTurnOrder();
}

ETurnState FRoundEndState::NextState()
{
	return ETurnState::ERoundStartState;
}

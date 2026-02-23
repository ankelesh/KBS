#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/RoundStartState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/TacTurnOrder.h"

void FRoundStartState::Enter()
{
	IncrementRound();
	ReloadTurnOrder();
	UE_LOG(LogKBSTurn, Log, TEXT("=== Round %d start === (%d units)"),
		ParentTurnSubsystem->GetCurrentRound(), GetTurnOrder()->GetRemainingUnits().Num());
}

void FRoundStartState::Exit()
{
	BroadcastRoundStart();
}

ETurnState FRoundStartState::NextState()
{
	return ETurnState::ETurnStartState;
}

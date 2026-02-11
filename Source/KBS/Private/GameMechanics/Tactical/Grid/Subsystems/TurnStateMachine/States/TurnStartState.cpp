#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/TurnStartState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/TacTurnOrder.h"
#include "GameMechanics/Units/Unit.h"

void FTurnStartState::Enter()
{
	GetTurnOrder()->Advance();
	BroadcastTurnStart();
	if (!GetTurnOrder()->Empty())
		GetTurnOrder()->GetCurrentUnit()->HandleTurnStart();
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Current unit was empty on start of turn!"));
	}
}

ETurnState FTurnStartState::NextState()
{
	return ETurnState::EActionsProcessingState;
}

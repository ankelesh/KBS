#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/TurnStartState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/TacTurnOrder.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "GameMechanics/Units/Unit.h"

void FTurnStartState::Enter()
{
	GetTurnOrder()->Advance();
	if (!GetTurnOrder()->Empty())
	{
		AUnit* Unit = GetTurnOrder()->GetCurrentUnit();
		UE_LOG(LogKBSTurn, Log, TEXT("Turn start: %s"), *Unit->GetLogName());
		Unit->HandleTurnStart();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Current unit was empty on start of turn!"));
	}
	BroadcastTurnStart();
}

ETurnState FTurnStartState::NextState()
{
	return ETurnState::EActionsProcessingState;
}

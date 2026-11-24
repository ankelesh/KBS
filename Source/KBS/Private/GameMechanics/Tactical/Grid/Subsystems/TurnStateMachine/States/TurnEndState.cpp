#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/TurnEndState.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/TacTurnOrder.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"

void FTurnEndState::Enter()
{
	BroadcastTurnEnd();
	if (AUnit* Unit = GetTurnOrder()->GetCurrentUnit())
	{
		UE_LOG(LogKBSTurn, Log, TEXT("Turn end:   %s"), *Unit->GetLogName());
		Unit->HandleTurnEnd();
	}
	else
	{
		UE_LOG(LogKBSTurn, Log, TEXT("Turn end: acting unit is gone (died during its turn)"));
	}
}

ETurnState FTurnEndState::NextState()
{
	if (GetTurnOrder()->IsRoundEnd())
	{
		return ETurnState::ERoundEndState;
	}
	else
	{
		return ETurnState::ETurnStartState;
	}
}

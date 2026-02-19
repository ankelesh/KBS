#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/TurnEndState.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/TacTurnOrder.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"

void FTurnEndState::Enter()
{
	BroadcastTurnEnd();
	if (!GetTurnOrder()->Empty())
	{
		AUnit* Unit = GetTurnOrder()->GetCurrentUnit();
		UE_LOG(LogKBSTurn, Log, TEXT("Turn end:   %s"), *UnitLogName(Unit));
		Unit->HandleTurnEnd();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("On turn end current unit was null!"));
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

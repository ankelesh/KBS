#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/TurnStartState.h"

#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/TacTurnOrder.h"

void TurnStartState::Enter()
{
	GetTurnOrder()->Advance();
	BroadcastTurnStart();
}

void TurnStartState::Exit()
{

}

void TurnStartState::UnitClicked(AUnit* Unit)
{
}

void TurnStartState::AbilityClicked(UUnitAbilityInstance* Ability)
{
}

ETurnProcessingSubstate TurnStartState::CanReleaseState()
{
	return ETurnProcessingSubstate::EFreeState;
}

ETurnState TurnStartState::NextState()
{
	return ETurnState::EActionsProcessingState;
}

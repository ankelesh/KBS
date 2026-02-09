#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/RoundStartState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"

void FRoundStartState::Enter()
{
	ReloadTurnOrder();
}

void FRoundStartState::Exit()
{
	BroadcastRoundStart();
}

void FRoundStartState::UnitClicked(AUnit* Unit)
{
}

void FRoundStartState::AbilityClicked(UUnitAbilityInstance* Ability)
{
}

ETurnProcessingSubstate FRoundStartState::CanReleaseState()
{
	return ETurnProcessingSubstate::EFreeState;
}

ETurnState FRoundStartState::NextState()
{
	return ETurnState::ETurnStartState;
}

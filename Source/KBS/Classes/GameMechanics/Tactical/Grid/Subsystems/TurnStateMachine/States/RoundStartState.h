#pragma once
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/TacTurnState.h"

class FRoundStartState : public FTacTurnState
{
public:
	explicit FRoundStartState(UTacTurnSubsystem* Parent)
		: FTacTurnState(Parent, ETurnProcessingSubstate::EFreeState) {}
	virtual void Enter() override;
	virtual void Exit() override;
	virtual ETurnState NextState() override;
};

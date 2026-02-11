#pragma once
#include "TacTurnState.h"

class FTurnStartState : public FTacTurnState
{
public:
	explicit FTurnStartState(UTacTurnSubsystem* Parent) : FTacTurnState(Parent, ETurnProcessingSubstate::EProcessingEndState){}
	virtual void Enter() override;
	virtual ETurnState NextState() override;
};

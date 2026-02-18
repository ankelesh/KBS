#pragma once
#include "TacTurnState.h"

class FTurnEndState : public FTacTurnState
{
public:
	explicit FTurnEndState(UTacTurnSubsystem* Parent) : FTacTurnState(Parent, ETurnProcessingSubstate::EFreeState){}
	virtual void Enter() override;
	virtual ETurnState NextState() override;
};

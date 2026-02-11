#pragma once
#include "TacTurnState.h"

class FBattleEndState : public FTacTurnState
{
public:
	explicit FBattleEndState(UTacTurnSubsystem* Parent) : FTacTurnState(Parent, ETurnProcessingSubstate::EProcessingEndState){}
	virtual void Enter() override;
	virtual void Exit() override;
	virtual ETurnState NextState() override;
};

#pragma once
#include "TacTurnState.h"


class FBattleInitializationState : public FTacTurnState
{
public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual ETurnProcessingSubstate CanReleaseState() override;
	virtual ETurnState NextState() override {return ETurnState::ERoundStartState;};
	FBattleInitializationState();
	
protected:
	void PrepareUnitStats();
};



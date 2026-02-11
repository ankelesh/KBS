#pragma once
#include "TacTurnState.h"


class FBattleInitializationState : public FTacTurnState
{
public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual ETurnState NextState() override {return ETurnState::ERoundStartState;};
	FBattleInitializationState(UTacTurnSubsystem* Parent) : FTacTurnState(Parent, ETurnProcessingSubstate::EFreeState){}
	
protected:
	void PrepareUnitStats();
};



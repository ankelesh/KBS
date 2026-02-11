#pragma once
#include "TacTurnState.h"

class FRoundEndState : public FTacTurnState
{
public:
	explicit FRoundEndState(UTacTurnSubsystem* Parent) : FTacTurnState(Parent, ETurnProcessingSubstate::EFreeState){}
	virtual void Enter() override;
	virtual ETurnState NextState() override;
};

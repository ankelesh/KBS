#pragma once
#include "TacTurnState.h"

class TurnStartState : public FTacTurnState
{
public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void UnitClicked(AUnit* Unit) override;
	virtual void AbilityClicked(UUnitAbilityInstance* Ability) override;
	virtual ETurnProcessingSubstate CanReleaseState() override;
	virtual ETurnState NextState() override;
};

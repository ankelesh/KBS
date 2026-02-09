#pragma once
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/TacTurnState.h"

class FRoundStartState : public FTacTurnState
{
public:
	friend class UTacTurnSubsystem;
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void UnitClicked(AUnit* Unit) override;
	virtual void AbilityClicked(UUnitAbilityInstance* Ability) override;
	virtual ETurnProcessingSubstate CanReleaseState() override;
	virtual ETurnState NextState() override;
};

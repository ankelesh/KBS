#pragma once
#include "TacTurnState.h"
#include "GameplayTypes/GridCoordinates.h"

class FActionsProcessingState : public FTacTurnState
{
public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void UnitClicked(AUnit* Unit) override;
	virtual void CellClicked(FTacCoordinates Cell) override;
	virtual void AbilityClicked(UUnitAbilityInstance* Ability) override;
	virtual void OnPresentationComplete() override;
	virtual ETurnState NextState() override;

	explicit FActionsProcessingState(UTacTurnSubsystem* Parent) : FTacTurnState(
		Parent, ETurnProcessingSubstate::EFreeState), bBattleEnded(false){}

private:
	void ExecuteAbilityOnTarget(FTacCoordinates TargetCell);
	void CheckAbilitiesAndSetupTurn();

	bool bBattleEnded = false;
};

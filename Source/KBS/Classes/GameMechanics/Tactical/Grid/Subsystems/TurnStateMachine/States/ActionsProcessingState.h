#pragma once
#include "TacTurnState.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/AbilityTypes.h"

class FActionsProcessingState : public FTacTurnState
{
public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void UnitClicked(AUnit* Unit) override;
	virtual void CellClicked(FTacCoordinates Cell) override;
	virtual void AbilityClicked(UUnitAbilityInstance* Ability) override;
	virtual void OnPresentationComplete() override;
	virtual ETurnProcessingSubstate CanReleaseState() override;
	virtual ETurnState NextState() override;
	FActionsProcessingState() : TurnProcessing(ETurnProcessingSubstate::EAwaitingInputState){}
private:
	void ExecuteAbilityOnTarget(FTacCoordinates TargetCell);
	void RefreshForNextAction();
	void HandleAbilityComplete(EAbilityTurnAction TurnAction);

	ETurnProcessingSubstate TurnProcessing;
	bool bTurnEnded = false;
	EAbilityTurnAction PendingTurnAction = EAbilityTurnAction::EndTurn;
	
};

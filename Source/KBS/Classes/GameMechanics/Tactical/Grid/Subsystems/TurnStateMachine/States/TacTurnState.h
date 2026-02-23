#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/GridCoordinates.h"

class FTacTurnOrder;

UENUM(BlueprintType)
enum class ETurnState : uint8
{
	// Battle loop
	EBattleInitializationState,
	// Main loop
	ERoundStartState,
	ETurnStartState,
	EActionsProcessingState,
	ETurnEndState,
	ERoundEndState,
	EBattleEndState
};

UENUM(BlueprintType)
enum class ETurnProcessingSubstate : uint8
{
	EFreeState, // can release state
	EAwaitingPresentationState, // can release, but needs presentation to end
	EAwaitingInputState, // can not proceed until input
	EProcessingEndState
};
class AUnit;
class UUnitAbilityInstance;
class UTacTurnSubsystem;
class FTacTurnOrder;
class UTacAICombatService;
class FTacTurnState
{
public:
	friend class UTacTurnSubsystem;
	virtual ~FTacTurnState() = default;
	explicit FTacTurnState(UTacTurnSubsystem* Parent, ETurnProcessingSubstate Substate = ETurnProcessingSubstate::EFreeState)
		: TurnProcessing(Substate), ParentTurnSubsystem(Parent) {}
	virtual void Enter();
	virtual void Exit();
	virtual void UnitClicked(AUnit * Unit) {}
	virtual void CellClicked(FTacCoordinates Cell) {}
	virtual void AbilityClicked(UUnitAbilityInstance * Ability) {}
	virtual void OnPresentationComplete() {}
	void SetParentTurnSubsystem(UTacTurnSubsystem * Parent) { ParentTurnSubsystem = Parent; };
	virtual ETurnProcessingSubstate CanReleaseState(){ return TurnProcessing; };
	virtual ETurnState NextState() = 0;
protected:
	FTacTurnOrder* GetTurnOrder();
	void ReloadTurnOrder();
	void IncrementRound();
	void BroadcastRoundStart();
	void BroadcastRoundEnd();
	void BroadcastTurnEnd();
	void BroadcastTurnStart();
	void BroadcastBattleEnd();
	UTacAICombatService* GetAIService();
	bool CheckWinCondition() const;
	ETurnProcessingSubstate TurnProcessing = ETurnProcessingSubstate::EFreeState;
	UTacTurnSubsystem* ParentTurnSubsystem;
};

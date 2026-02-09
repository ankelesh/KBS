#pragma once
#include "CoreMinimal.h"


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
	EAwaitingInputState // can not proceed until input
};
class AUnit;
class UUnitAbilityInstance;
class UTacTurnSubsystem;
class FTacTurnOrder;
class FTacTurnState
{
public:
	friend class UTacTurnSubsystem;
	virtual ~FTacTurnState() = default;
	virtual void Enter();
	virtual void Exit();
	virtual void UnitClicked(AUnit * Unit) {}
	virtual void CellClicked(FTacCoordinates Cell) {}
	virtual void AbilityClicked(UUnitAbilityInstance * Ability) {}
	virtual void OnPresentationComplete() {}
	virtual ETurnProcessingSubstate CanReleaseState(){ return TurnProcessing; };
	virtual ETurnState NextState() = 0;
protected:
	FTacTurnOrder* GetTurnOrder();
	void ReloadTurnOrder();
	void BroadcastRoundStart();
	void BroadcastRoundEnd();
	void BroadcastTurnEnd();
	void BroadcastTurnStart();
	
	ETurnProcessingSubstate TurnProcessing = ETurnProcessingSubstate::EFreeState;
	UTacTurnSubsystem* ParentTurnSubsystem;
};

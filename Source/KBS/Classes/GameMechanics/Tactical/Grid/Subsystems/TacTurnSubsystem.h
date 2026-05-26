// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TurnStateMachine/TacTurnOrder.h"
#include "TurnStateMachine/States/TacTurnState.h"
#include "TacCategoryLogger.h"
#include "TacTurnSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogKBSTurn, Log, All);

class UTacGridSubsystem;
class UTacAICombatService;
class UBattleTeam;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStart, int32, Turn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundEnd, int32, Turn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStart, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnEnd, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleEnd, UBattleTeam*, WinnerTeam);
class FTacTurnState;
class AUnit;
class UUnitAbility;
struct FTacCoordinates;

UCLASS()
class KBS_API UTacTurnSubsystem : public UWorldSubsystem
{
public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	GENERATED_BODY()

public:
	friend class FTacTurnState;

	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Delegates
	FOnRoundStart OnRoundStart;
	FOnRoundEnd OnRoundEnd;
	FOnTurnStart OnTurnStart;
	FOnTurnEnd OnTurnEnd;
	FOnBattleEnd OnBattleEnd;

	// Battle control
	void StartBattle();

	// Turn order operations
	void Wait();
	void RegisterSummonedUnit(AUnit* Unit);

	// Input forwarding
	void UnitClicked(AUnit* Unit);
	void CellClicked(FTacCoordinates Cell);
	void AbilityClicked(UUnitAbility* Ability);

	int32 GetCurrentRound() const { return CurrentRound; }
	UTacAICombatService* GetAICombatService() const { return AICombatService; }

	// Turn order queries
	AUnit* GetCurrentUnit() const;
	TArray<AUnit*> GetRemainingUnits(int32 TruncList = -1) const;
	int32 GetUnitInitiative(AUnit* Unit) const;

	UFUNCTION(BlueprintCallable)
	void GridAvailable();
	UFUNCTION(BlueprintCallable)
	void OnPresentationComplete();

private:
	struct FTransitionRecord
	{
		ETurnState From;
		ETurnState To;
		FString    Trigger;
		int32      Round;
		FString    UnitName;
	};

	UFUNCTION()
	void HandleUnitDied(AUnit* Unit);

	void InitializeStates();
	void TransitionToState(ETurnState NextState);
	void AttemptTransition();
	void AttemptTransition(int32 Depth);

	void RecordPhase(ETurnState State);
	void RecordTransition(ETurnState From, ETurnState To);
	void RecordEvent(const FString& Event);
	void DumpInfiniteLoopDiagnostics() const;

	void ReloadTurnOrder();
	void BroadcastRoundStart();
	void BroadcastRoundEnd();
	void BroadcastTurnStart();
	void BroadcastTurnEnd();
	void BroadcastBattleEnd();

	static constexpr int32 KPhaseHistorySize     = 20;
	static constexpr int32 KTransitionRecordSize = 10;
	static constexpr int32 KEventHistorySize     = 20;

	TUniquePtr<FTacTurnOrder> TurnOrder;
	TMap<ETurnState, TUniquePtr<FTacTurnState>> States;
	TUniquePtr<FTacCategoryLogger> TurnLogger;
	TUniquePtr<FTacCategoryLogger> AILogger;
	FTacTurnState* CurrentState = nullptr;
	ETurnState CurrentStateEnum = ETurnState::EBattleInitializationState;
	int32 CurrentRound = 0;
	UTacGridSubsystem* GridSubsystem = nullptr;
	UPROPERTY()
	TObjectPtr<UTacAICombatService> AICombatService;

	TArray<ETurnState>        PhaseHistory;
	TArray<FTransitionRecord> TransitionHistory;
	TArray<FString>           EventHistory;
	int32                     PhaseHistoryIdx      = 0;
	int32                     TransitionHistoryIdx = 0;
	int32                     EventHistoryIdx      = 0;
	FString                   PendingTrigger;
};

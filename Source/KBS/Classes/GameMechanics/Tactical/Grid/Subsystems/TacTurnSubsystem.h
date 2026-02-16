// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TurnStateMachine/TacTurnOrder.h"
#include "TacTurnSubsystem.generated.h"

class UTacGridSubsystem;
class UBattleTeam;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStart, int32, Turn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundEnd, int32, Turn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStart, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnEnd, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleEnd, UBattleTeam*, WinnerTeam);
class FTacTurnState;
class AUnit;
class UUnitAbilityInstance;
struct FTacCoordinates;
enum class ETurnState : uint8;

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

	// Input forwarding
	void UnitClicked(AUnit* Unit);
	void CellClicked(FTacCoordinates Cell);
	void AbilityClicked(UUnitAbilityInstance* Ability);

	int32 GetCurrentRound() const { return CurrentRound; }

	// Turn order queries
	AUnit* GetCurrentUnit() const;
	TArray<AUnit*> GetRemainingUnits(int32 TruncList = -1) const;
	int32 GetUnitInitiative(AUnit* Unit) const;

	UFUNCTION(BlueprintCallable)
	void GridAvailable(UTacGridSubsystem* Grid);
	UFUNCTION(BlueprintCallable)
	void OnPresentationComplete();

private:
	void InitializeStates();
	void TransitionToState(ETurnState NextState);
	void AttemptTransition();

	void ReloadTurnOrder();
	void BroadcastRoundStart();
	void BroadcastRoundEnd();
	void BroadcastTurnStart();
	void BroadcastTurnEnd();
	void BroadcastBattleEnd();

	TUniquePtr<FTacTurnOrder> TurnOrder;
	TMap<ETurnState, TUniquePtr<FTacTurnState>> States;
	FTacTurnState* CurrentState = nullptr;
	int32 CurrentRound;
	UTacGridSubsystem* GridSubsystem = nullptr;
};

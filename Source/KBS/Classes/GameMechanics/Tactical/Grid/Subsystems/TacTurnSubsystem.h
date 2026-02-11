// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TacTurnSubsystem.generated.h"

class UBattleTeam;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStart, int32, Turn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundEnd, int32, Turn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStart, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnEnd, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleEnd, UBattleTeam*, WinnerTeam);


class FTacTurnOrder;
class FTacTurnState;
class AUnit;
class UUnitAbilityInstance;
struct FTacCoordinates;
enum class ETurnState : uint8;

UCLASS()
class KBS_API UTacTurnSubsystem : public UWorldSubsystem
{
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

private:
	void InitializeStates();
	void TransitionToState(ETurnState NextState);
	void AttemptTransition();
	void OnPresentationComplete();

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
};

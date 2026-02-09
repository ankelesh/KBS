// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TacTurnSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStart, int32, Turn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundEnd, int32, Turn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStart, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnEnd, AUnit*, Unit);

class FTacTurnOrder;
class FTacTurnState;
UCLASS()
class KBS_API UTacTurnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	friend class FTacTurnState;
	// Delegates
	FOnRoundStart OnRoundStart;
	FOnRoundEnd OnRoundEnd;
	FOnTurnStart OnTurnStart;
	FOnTurnEnd OnTurnEnd;
	
	
	int32 GetCurrentRound() const { return CurrentRound; }
private:
	void ReloadTurnOrder();
	void BroadcastRoundStart();
	void BroadcastRoundEnd();
	void BroadcastTurnStart();
	void BroadcastTurnEnd();
	
	TUniquePtr<FTacTurnOrder> TurnOrder;
	int32 CurrentRound;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameplayTypes/GridCoordinates.h"
#include "TacAbilityExecutorService.generated.h"

class UUnitAbilityInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityExecuting, UUnitAbilityInstance*, Ability, FTacCoordinates, TargetCell);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAbilityCompleted, UUnitAbilityInstance*, Ability, FTacCoordinates, TargetCell, const FAbilityResult&, Result);

UCLASS()
class KBS_API UTacAbilityExecutorService : public UObject
{
	GENERATED_BODY()

public:
	// Strong guarantee: validates before execution
	FAbilityResult CheckAndExecute(UUnitAbilityInstance* Ability, FTacCoordinates TargetCell);

	// Weak guarantee: attempts execution, reports input failure
	FAbilityResult Execute(UUnitAbilityInstance* Ability, FTacCoordinates TargetCell);

	// Delegates
	UPROPERTY(BlueprintAssignable)
	FOnAbilityExecuting OnAbilityExecuting;   // Broadcast post-check, pre-exec

	UPROPERTY(BlueprintAssignable)
	FOnAbilityCompleted OnAbilityCompleted;   // Broadcast post-exec
};
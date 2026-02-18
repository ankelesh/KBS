// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/GridCoordinates.h"
#include "TacAICombatService.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogKBSAI, Log, All);

class AUnit;
class UTacGridSubsystem;
class UTacCombatSubsystem;
class UUnitAbilityInstance;

USTRUCT()
struct FAiDecision
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UUnitAbilityInstance> AbilityToUse = nullptr;

	FTacCoordinates TargetCell;
	bool bHasDecision = false;
};

UCLASS()
class KBS_API UTacAICombatService : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(UTacGridSubsystem* InGridSubsystem, UTacCombatSubsystem* InCombatSubsystem);
	FAiDecision ThinkOverNextAction(AUnit* Unit) const;

private:
	bool TryDecideAttack(AUnit* Unit, FAiDecision& OutDecision) const;
	bool TryDecideMove(AUnit* Unit, FAiDecision& OutDecision) const;
	void DecideWait(AUnit* Unit, FAiDecision& OutDecision) const;

	// Returns move cell from ValidCells that minimizes distance to the nearest enemy
	FTacCoordinates PickMoveTowardEnemy(AUnit* Unit, const TArray<FTacCoordinates>& MoveCells) const;

	UPROPERTY()
	TObjectPtr<UTacGridSubsystem> GridSubsystem;
	UPROPERTY()
	TObjectPtr<UTacCombatSubsystem> CombatSubsystem;
};

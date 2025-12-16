#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIControllerComponent.generated.h"

class AUnit;
class ATacBattleGrid;
class UGridDataManager;
class UGridMovementComponent;
class UGridTargetingComponent;
class UAbilityExecutorComponent;

/**
 * AI decision-making component for enemy units on the tactical grid
 */
UCLASS()
class KBS_API UAIControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIControllerComponent();

	/**
	 * Initialize component with grid and component references
	 */
	void Initialize(ATacBattleGrid* InGrid, UGridDataManager* InDataManager,
	                UGridMovementComponent* InMovement, UGridTargetingComponent* InTargeting,
	                UAbilityExecutorComponent* InAbilityExecutor);

	/**
	 * Main AI entry point - evaluate attack/move/skip priority chain
	 */
	void ExecuteAITurn(AUnit* AIUnit);

private:
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;

	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;

	UPROPERTY()
	TObjectPtr<UGridMovementComponent> MovementComponent;

	UPROPERTY()
	TObjectPtr<UGridTargetingComponent> TargetingComponent;

	UPROPERTY()
	TObjectPtr<UAbilityExecutorComponent> AbilityExecutor;

	/**
	 * Attempt to attack valid target with highest expected damage
	 */
	bool TryExecuteAttack(AUnit* AIUnit);

	/**
	 * Move toward closest enemy if no attack available
	 */
	bool TryMoveTowardEnemy(AUnit* AIUnit);

	/**
	 * Select target with highest damage preview using DamageCalculator
	 */
	AUnit* FindBestTarget(AUnit* AIUnit, const TArray<AUnit*>& PotentialTargets);

	/**
	 * Find valid move cell closest to target enemy
	 */
	FIntPoint FindBestMovePosition(AUnit* AIUnit, AUnit* TargetEnemy);
};

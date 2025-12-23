#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIControllerComponent.generated.h"
class AUnit;
class UGridDataManager;
class UGridMovementComponent;
class UGridTargetingComponent;
class UAbilityExecutorComponent;
class UTurnManagerComponent;
UCLASS()
class KBS_API UAIControllerComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UAIControllerComponent();
	void Initialize(UGridDataManager* InDataManager,
	                UGridMovementComponent* InMovement, UGridTargetingComponent* InTargeting,
	                UAbilityExecutorComponent* InAbilityExecutor, UTurnManagerComponent* InTurnManager);
	void ExecuteAITurn(AUnit* AIUnit);
private:
	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;
	UPROPERTY()
	TObjectPtr<UGridMovementComponent> MovementComponent;
	UPROPERTY()
	TObjectPtr<UGridTargetingComponent> TargetingComponent;
	UPROPERTY()
	TObjectPtr<UAbilityExecutorComponent> AbilityExecutor;
	UPROPERTY()
	TObjectPtr<UTurnManagerComponent> TurnManager;
	bool TryExecuteAttack(AUnit* AIUnit);
	bool TryMoveTowardEnemy(AUnit* AIUnit);
	AUnit* FindBestTarget(AUnit* AIUnit, const TArray<AUnit*>& PotentialTargets);
	FIntPoint FindBestMovePosition(AUnit* AIUnit, AUnit* TargetEnemy);
};

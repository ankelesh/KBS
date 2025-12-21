#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TacGridInputRouter.generated.h"

class AUnit;
class ATacBattleGrid;
class UGridDataManager;
class UGridMovementComponent;
class UGridTargetingComponent;
class UTurnManagerComponent;
class UGridInputLockComponent;
enum class EBattleLayer : uint8;

/**
 * Handles input routing for tactical grid clicks
 * Routes player clicks to appropriate actions based on game state
 */
UCLASS()
class KBS_API UTacGridInputRouter : public UActorComponent
{
	GENERATED_BODY()

public:
	UTacGridInputRouter();

	/**
	 * Initialize component with required references
	 */
	void Initialize(
		ATacBattleGrid* InGrid,
		UGridDataManager* InDataManager,
		UGridMovementComponent* InMovementComponent,
		UGridTargetingComponent* InTargetingComponent,
		UTurnManagerComponent* InTurnManager,
		UGridInputLockComponent* InInputLockComponent
	);

	/**
	 * Handle grid click from player
	 * Routes to target selection, movement, or inspection based on context
	 */
	void HandleGridClick(FKey ButtonPressed);

private:
	// Component references
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;

	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;

	UPROPERTY()
	TObjectPtr<UGridMovementComponent> MovementComponent;

	UPROPERTY()
	TObjectPtr<UGridTargetingComponent> TargetingComponent;

	UPROPERTY()
	TObjectPtr<UTurnManagerComponent> TurnManager;

	UPROPERTY()
	TObjectPtr<class UGridInputLockComponent> InputLockComponent;

	/**
	 * Get cell under mouse cursor
	 */
	bool GetCellUnderMouse(int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const;
};

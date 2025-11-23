#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridTargetingComponent.generated.h"

class AUnit;
class ATacBattleGrid;
class UBattleTeam;
class UGridDataManager;
enum class EBattleLayer : uint8;
enum class ETargetReach : uint8;

/**
 * Handles targeting validation and enemy detection on the tactical grid
 */
UCLASS()
class KBS_API UGridTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGridTargetingComponent();

	/**
	 * Initialize component with grid reference
	 */
	void Initialize(ATacBattleGrid* InGrid, UGridDataManager* InDataManager);

	/**
	 * Get all valid target cells based on reach type
	 */
	TArray<FIntPoint> GetValidTargetCells(AUnit* Unit, ETargetReach Reach) const;

	/**
	 * Get all valid target units based on reach type
	 */
	TArray<AUnit*> GetValidTargetUnits(AUnit* Unit, ETargetReach Reach) const;

private:
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;

	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;

	/**
	 * Get adjacent enemy cells (8 directions)
	 */
	void GetClosestEnemyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;

	/**
	 * Get flank enemy cells (closest to center in adjacent columns)
	 */
	void GetFlankTargetCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;

	/**
	 * Get all enemy cells on the grid
	 */
	void GetAnyEnemyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;
};

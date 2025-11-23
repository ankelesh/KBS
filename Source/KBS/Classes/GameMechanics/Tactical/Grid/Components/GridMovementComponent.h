#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridMovementComponent.generated.h"

class AUnit;
class ATacBattleGrid;
class UBattleTeam;
class UGridDataManager;
enum class EBattleLayer : uint8;

/**
 * Handles unit movement validation and execution on the tactical grid
 */
UCLASS()
class KBS_API UGridMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGridMovementComponent();

	/**
	 * Initialize component with grid reference
	 */
	void Initialize(ATacBattleGrid* InGrid, UGridDataManager* InDataManager);

	/**
	 * Get all valid cells a unit can move to
	 */
	TArray<FIntPoint> GetValidMoveCells(AUnit* Unit) const;

	/**
	 * Move a unit to target cell (handles swapping and flank entry)
	 */
	bool MoveUnit(AUnit* Unit, int32 TargetRow, int32 TargetCol);

private:
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;

	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;

	/**
	 * Get valid adjacent movement cells for ground units
	 */
	void GetAdjacentMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;

	/**
	 * Get special flank movement cells (closest enemy flank only)
	 */
	void GetFlankMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;

	/**
	 * Get all valid cells for air units (can move anywhere valid)
	 */
	void GetAirMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;
};

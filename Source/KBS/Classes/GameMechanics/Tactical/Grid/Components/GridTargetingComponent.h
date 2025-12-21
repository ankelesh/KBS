#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridTargetingComponent.generated.h"

class AUnit;
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
	 * Initialize component with data manager reference
	 */
	void Initialize(UGridDataManager* InDataManager);

	/**
	 * Get all valid target cells for unit's current ability
	 * Automatically extracts reach from unit's active ability
	 */
	TArray<FIntPoint> GetValidTargetCells(AUnit* Unit) const;

	/**
	 * Get all valid target cells based on reach type
	 * @param bUseFlankTargeting - Override to use flank targeting (only for ClosestEnemies)
	 */
	TArray<FIntPoint> GetValidTargetCells(AUnit* Unit, ETargetReach Reach, bool bUseFlankTargeting = false) const;

	/**
	 * Get all valid target units for unit's current ability
	 * Automatically extracts reach from unit's active ability
	 */
	TArray<AUnit*> GetValidTargetUnits(AUnit* Unit) const;

	/**
	 * Get all valid target units based on reach type
	 * @param bUseFlankTargeting - Override to use flank targeting (only for ClosestEnemies)
	 * @param bIncludeDeadUnits - If false (default), filters out dead units
	 */
	TArray<AUnit*> GetValidTargetUnits(AUnit* Unit, ETargetReach Reach, bool bUseFlankTargeting = false, bool bIncludeDeadUnits = false) const;

	/**
	 * Resolve actual targets based on clicked cell and reach type
	 * Expands single-target click to multi-target based on weapon's TargetReach
	 * @param SourceUnit - Unit performing the ability
	 * @param ClickedCell - Cell that was clicked by player
	 * @param ClickedLayer - Layer of clicked cell
	 * @param Reach - TargetReach type to use for expansion
	 * @param AreaShape - Optional area shape for Area-type reaches
	 */
	TArray<AUnit*> ResolveTargetsFromClick(AUnit* SourceUnit, FIntPoint ClickedCell, EBattleLayer ClickedLayer, ETargetReach Reach, const struct FAreaShape* AreaShape = nullptr) const;

private:
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

	/**
	 * Get all friendly cells on the grid
	 */
	void GetAllFriendlyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;

	/**
	 * Get any friendly cells (same as all for now)
	 */
	void GetAnyFriendlyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;

	/**
	 * Get empty cells or cells with friendly units
	 */
	void GetEmptyCellsOrFriendly(AUnit* Unit, TArray<FIntPoint>& OutCells) const;

	/**
	 * Get all units in area shape centered on target cell
	 */
	TArray<AUnit*> GetUnitsInArea(FIntPoint CenterCell, EBattleLayer Layer, const struct FAreaShape& AreaShape) const;
};

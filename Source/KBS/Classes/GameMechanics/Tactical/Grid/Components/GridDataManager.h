#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameMechanics/Units/Unit.h"
#include "GridDataManager.generated.h"

class ATacBattleGrid;

USTRUCT()
struct FGridRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<AUnit>> Cells;

	FGridRow()
	{
		Cells.Init(nullptr, 5);
	}
};

/**
 * Manages grid data storage and unit placement
 * Owns the 2D arrays for ground and air layers
 */
UCLASS()
class KBS_API UGridDataManager : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Initialize grid layers and create teams
	 */
	void Initialize(ATacBattleGrid* InGrid);

	/**
	 * Place a unit in the grid
	 */
	bool PlaceUnit(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer, ATacBattleGrid* Grid);

	/**
	 * Get unit at specified cell
	 */
	AUnit* GetUnit(int32 Row, int32 Col, EBattleLayer Layer) const;

	/**
	 * Remove unit from specified cell
	 */
	bool RemoveUnit(int32 Row, int32 Col, EBattleLayer Layer, ATacBattleGrid* Grid);

	/**
	 * Get layer array (mutable)
	 */
	TArray<FGridRow>& GetLayer(EBattleLayer Layer);

	/**
	 * Get layer array (const)
	 */
	const TArray<FGridRow>& GetLayer(EBattleLayer Layer) const;

	/**
	 * Find unit position in grid
	 * Returns true if found, fills Row, Col, Layer with position
	 */
	bool GetUnitPosition(const AUnit* Unit, int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const;

	/**
	 * Flank state management
	 */
	bool IsUnitOnFlank(const AUnit* Unit) const;
	void SetUnitFlankState(AUnit* Unit, bool bOnFlank);
	FRotator GetUnitOriginalRotation(const AUnit* Unit) const;
	void SetUnitOriginalRotation(AUnit* Unit, const FRotator& Rotation);

	/**
	 * Grid query methods (read-only, delegated to Grid)
	 */
	bool IsValidCell(int32 Row, int32 Col, EBattleLayer Layer) const;
	bool IsFlankCell(int32 Row, int32 Col) const;
	bool IsRestrictedCell(int32 Row, int32 Col) const;
	FVector GetCellWorldLocation(int32 Row, int32 Col, EBattleLayer Layer) const;

	/**
	 * Team query methods
	 */
	UBattleTeam* GetTeamForUnit(AUnit* Unit) const;
	UBattleTeam* GetEnemyTeam(AUnit* Unit) const;
	UBattleTeam* GetAttackerTeam() const { return AttackerTeam; }
	UBattleTeam* GetDefenderTeam() const { return DefenderTeam; }

	/**
	 * Get units from specified team (Blueprint-callable)
	 * @param bIsAttacker - true for attacker team, false for defender team
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid|Teams")
	TArray<AUnit*> GetUnitsFromTeam(bool bIsAttacker) const;

	/**
	 * Cell state query methods
	 */
	TArray<FIntPoint> GetEmptyCells(EBattleLayer Layer) const;
	TArray<FIntPoint> GetOccupiedCells(EBattleLayer Layer, UBattleTeam* Team) const;
	bool IsCellOccupied(int32 Row, int32 Col, EBattleLayer Layer) const;
	TArray<FIntPoint> GetValidPlacementCells(EBattleLayer Layer) const;

private:
	UPROPERTY()
	TArray<FGridRow> GroundLayer;

	UPROPERTY()
	TArray<FGridRow> AirLayer;

	// External unit state tracking
	TMap<AUnit*, bool> UnitFlankStates;
	TMap<AUnit*, FRotator> UnitOriginalRotations;

	// Grid and team references for query delegation
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;

	UPROPERTY()
	TObjectPtr<UBattleTeam> AttackerTeam;

	UPROPERTY()
	TObjectPtr<UBattleTeam> DefenderTeam;

	FVector GridWorldLocation;
};

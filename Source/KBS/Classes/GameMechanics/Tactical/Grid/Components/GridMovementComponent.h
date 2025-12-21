#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GridMovementComponent.generated.h"

class AUnit;
class ATacBattleGrid;
class UBattleTeam;
class UGridDataManager;
enum class EBattleLayer : uint8;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnUnitFlankStateChanged, AUnit* /*Unit*/, bool /*bEntering*/, FIntPoint /*Cell*/);

USTRUCT()
struct FMovementInterpData
{
	GENERATED_BODY()

	FVector StartLocation = FVector::ZeroVector;
	FVector TargetLocation = FVector::ZeroVector;
	FVector Direction = FVector::ZeroVector;
	float ElapsedTime = 0.0f;
	float Duration = 0.3f;
	bool bNeedsFinalRotation = false;
	FRotator FinalRotation = FRotator::ZeroRotator;
	ETeamSide UnitTeamSide = ETeamSide::Attacker;
};

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

	/** Fired when unit enters or exits a flank cell */
	FOnUnitFlankStateChanged OnUnitFlankStateChanged;

	/**
	 * Get all valid cells a unit can move to
	 */
	TArray<FIntPoint> GetValidMoveCells(AUnit* Unit) const;

	/**
	 * Move a unit to target cell (handles swapping and flank entry)
	 */
	bool MoveUnit(AUnit* Unit, int32 TargetRow, int32 TargetCol);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Model orientation constants (models are Y-forward, UE standard is X-forward)
	static constexpr float ModelForwardOffset = -90.0f; // Y-forward correction (180° flip from initial 90°)

	// Default team orientations (already account for Y-forward models)
	static constexpr float AttackerDefaultYaw = 0.0f;   // Face right (+X direction)
	static constexpr float DefenderDefaultYaw = 180.0f; // Face left (-X direction)

private:
	// Grid reference - ONLY used for event broadcasting (OnMovementComplete)
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;

	// Primary data access - all queries go through DataManager
	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;

	TMap<TObjectPtr<AUnit>, FMovementInterpData> UnitsBeingMoved;

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

	void StartMovementInterpolation(AUnit* Unit, FVector StartLocation, FVector TargetLocation, FVector Direction, float Distance, float Speed, int32 TargetRow, int32 TargetCol);

	/**
	 * Calculate default orientation for a unit standing in a specific cell
	 */
	FRotator CalculateDefaultCellOrientation(int32 Row, int32 Col, ETeamSide TeamSide) const;

	/**
	 * Find closest non-flank cell from a flank cell position
	 */
	FIntPoint FindClosestNonFlankCell(int32 FlankRow, int32 FlankCol) const;

	/**
	 * Check if unit can enter specific enemy flank cell based on position rules
	 */
	bool CanEnterFlankCell(const FIntPoint& UnitPos, const FIntPoint& FlankCell, UBattleTeam* UnitTeam) const;

	/**
	 * Check if two cells are adjacent (Manhattan distance = 1, no diagonal)
	 */
	bool IsAdjacentCell(const FIntPoint& CellA, const FIntPoint& CellB) const;

	/**
	 * Check if cell position is a flank cell
	 */
	bool IsFlankCell(const FIntPoint& Cell) const;

	/**
	 * Apply flank rotation when unit enters flank cell
	 */
	void ApplyFlankRotation(AUnit* Unit, int32 Row, int32 Col);

	/**
	 * Restore original rotation when unit exits flank cell
	 */
	void RestoreOriginalRotation(AUnit* Unit);
};

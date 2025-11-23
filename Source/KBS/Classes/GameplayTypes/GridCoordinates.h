#pragma once

#include "CoreMinimal.h"
#include "GameMechanics/Units/Unit.h"
#include "GridCoordinates.generated.h"

/**
 * Grid coordinate utilities and constants for tactical battle grid
 * Static helper struct for grid math and validation
 */
USTRUCT()
struct KBS_API FGridCoordinates
{
	GENERATED_BODY()

	// Grid constants
	static constexpr int32 GridSize = 5;
	static constexpr int32 TotalCells = 25;
	static constexpr float CellSize = 200.0f;
	static constexpr float AirLayerHeight = 500.0f;
	static constexpr int32 CenterRow = 2;
	static constexpr int32 ExcludedColLeft = 0;
	static constexpr int32 ExcludedColRight = 4;

	/**
	 * Check if a cell is within valid grid bounds and not restricted
	 */
	static bool IsValidCell(int32 Row, int32 Col);

	/**
	 * Check if a cell is a flank cell (columns 0 or 4, not center row)
	 */
	static bool IsFlankCell(int32 Row, int32 Col);

	/**
	 * Check if a cell is restricted (center row flank positions)
	 */
	static bool IsRestrictedCell(int32 Row, int32 Col);

	/**
	 * Convert cell coordinates to world location
	 */
	static FVector CellToWorldLocation(int32 Row, int32 Col, EBattleLayer Layer, const FVector& GridOrigin);

	/**
	 * Convert world location to cell coordinates
	 */
	static FIntPoint WorldLocationToCell(const FVector& WorldLocation, const FVector& GridOrigin);

	/**
	 * Get rotation for unit on flank cell (faces the single neighbor cell)
	 */
	static FRotator GetFlankRotation(int32 Row, int32 Col);
};

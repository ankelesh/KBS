#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Unit.h"
#include "GridCoordinates.generated.h"
USTRUCT()
struct KBS_API FGridCoordinates
{
	GENERATED_BODY()
	static constexpr int32 GridSize = 5;
	static constexpr int32 TotalCells = 25;
	static constexpr float CellSize = 200.0f;
	static constexpr float AirLayerHeight = 500.0f;
	static constexpr int32 CenterRow = 2;
	static constexpr int32 ExcludedColLeft = 0;
	static constexpr int32 ExcludedColRight = 4;
	static bool IsValidCell(int32 Row, int32 Col);
	static bool IsFlankCell(int32 Row, int32 Col);
	static bool IsRestrictedCell(int32 Row, int32 Col);
	static FVector CellToWorldLocation(int32 Row, int32 Col, EBattleLayer Layer, const FVector& GridOrigin);
	static FIntPoint WorldLocationToCell(const FVector& WorldLocation, const FVector& GridOrigin);
	static FRotator GetFlankRotation(int32 Row, int32 Col);
};

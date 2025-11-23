#include "GameplayTypes/GridCoordinates.h"

bool FGridCoordinates::IsValidCell(int32 Row, int32 Col)
{
	if (Row < 0 || Row >= GridSize || Col < 0 || Col >= GridSize)
	{
		return false;
	}

	if (Row == CenterRow && (Col == ExcludedColLeft || Col == ExcludedColRight))
	{
		return false;
	}

	return true;
}

bool FGridCoordinates::IsFlankCell(int32 Row, int32 Col)
{
	if (Row == CenterRow)
	{
		return false;
	}

	return Col == ExcludedColLeft || Col == ExcludedColRight;
}

bool FGridCoordinates::IsRestrictedCell(int32 Row, int32 Col)
{
	return Row == CenterRow && (Col == ExcludedColLeft || Col == ExcludedColRight);
}

FVector FGridCoordinates::CellToWorldLocation(int32 Row, int32 Col, EBattleLayer Layer, const FVector& GridOrigin)
{
	const float X = Col * CellSize + CellSize * 0.5f;
	const float Y = Row * CellSize + CellSize * 0.5f;
	const float Z = (Layer == EBattleLayer::Air) ? AirLayerHeight : 0.0f;

	return GridOrigin + FVector(X, Y, Z);
}

FIntPoint FGridCoordinates::WorldLocationToCell(const FVector& WorldLocation, const FVector& GridOrigin)
{
	const FVector LocalPos = WorldLocation - GridOrigin;

	const int32 Col = FMath::FloorToInt(LocalPos.X / CellSize);
	const int32 Row = FMath::FloorToInt(LocalPos.Y / CellSize);

	return FIntPoint(FMath::Clamp(Col, 0, GridSize - 1), FMath::Clamp(Row, 0, GridSize - 1));
}

FRotator FGridCoordinates::GetFlankRotation(int32 Row, int32 Col)
{
	if (!IsFlankCell(Row, Col))
	{
		return FRotator::ZeroRotator;
	}

	// Left flank (col 0): face right toward center (+X direction in grid)
	if (Col == ExcludedColLeft)
	{
		return FRotator(0.0f, -90.0f, 0.0f);
	}
	// Right flank (col 4): face left toward center (-X direction in grid)
	else if (Col == ExcludedColRight)
	{
		return FRotator(0.0f, 90.0f, 0.0f);
	}

	return FRotator::ZeroRotator;
}

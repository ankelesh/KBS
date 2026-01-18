#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Units/Unit.h"

bool FTacCoordinates::IsValidCell(int32 Row, int32 Col)
{
	if (Row < 0 || Row >= FGridConstants::GridSize || Col < 0 || Col >= FGridConstants::GridSize)
	{
		return false;
	}
	if (Row == FGridConstants::CenterRow && (Col == FGridConstants::ExcludedColLeft || Col == FGridConstants::ExcludedColRight))
	{
		return false;
	}
	return true;
}
bool FTacCoordinates::IsFlankCell(int32 Row, int32 Col)
{
	if (Row == FGridConstants::CenterRow)
	{
		return false;
	}
	return Col == FGridConstants::ExcludedColLeft || Col == FGridConstants::ExcludedColRight;
}
bool FTacCoordinates::IsRestrictedCell(int32 Row, int32 Col)
{
	return Row == FGridConstants::CenterRow && (Col == FGridConstants::ExcludedColLeft || Col == FGridConstants::ExcludedColRight);
}
FVector FTacCoordinates::CellToWorldLocation(int32 Row, int32 Col, EBattleLayer Layer, const FVector& GridOrigin, float CellSize, float AirLayerHeight)
{
	const float X = Col * CellSize + CellSize * 0.5f;
	const float Y = Row * CellSize + CellSize * 0.5f;
	const float Z = (Layer == EBattleLayer::Air) ? AirLayerHeight : 0.0f;
	return GridOrigin + FVector(X, Y, Z);
}
FIntPoint FTacCoordinates::WorldLocationToCell(const FVector& WorldLocation, const FVector& GridOrigin, float CellSize)
{
	const FVector LocalPos = WorldLocation - GridOrigin;
	const int32 Col = FMath::FloorToInt(LocalPos.X / CellSize);
	const int32 Row = FMath::FloorToInt(LocalPos.Y / CellSize);
	return FIntPoint(FMath::Clamp(Col, 0, FGridConstants::GridSize - 1), FMath::Clamp(Row, 0, FGridConstants::GridSize - 1));
}
FRotator FTacCoordinates::GetFlankRotation(int32 Row, int32 Col)
{
	if (!IsFlankCell(Row, Col))
	{
		return FRotator::ZeroRotator;
	}
	if (Col == FGridConstants::ExcludedColLeft)
	{
		return FRotator(0.0f, -90.0f, 0.0f);
	}
	else if (Col == FGridConstants::ExcludedColRight)
	{
		return FRotator(0.0f, 90.0f, 0.0f);
	}
	return FRotator::ZeroRotator;
}

// Instance methods
bool FTacCoordinates::IsValidCell() const
{
	return IsValidCell(Row, Col);
}

bool FTacCoordinates::IsFlankCell() const
{
	return IsFlankCell(Row, Col);
}

bool FTacCoordinates::IsRestrictedCell() const
{
	return IsRestrictedCell(Row, Col);
}

FVector FTacCoordinates::ToWorldLocation(const FVector& GridOrigin, float CellSize, float AirLayerHeight) const
{
	return CellToWorldLocation(Row, Col, Layer, GridOrigin, CellSize, AirLayerHeight);
}

FRotator FTacCoordinates::GetFlankRotation() const
{
	return GetFlankRotation(Row, Col);
}

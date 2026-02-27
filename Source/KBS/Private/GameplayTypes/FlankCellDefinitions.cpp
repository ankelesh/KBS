#include "GameplayTypes/FlankCellDefinitions.h"
const TArray<int32> FFlankCellDefinitions::AttackerFlankColumns = {0, 1};
const TArray<int32> FFlankCellDefinitions::DefenderFlankColumns = {3, 4};
const TArray<int32> FFlankCellDefinitions::ClosestFlankColumns = {1, 3};
const TArray<int32> FFlankCellDefinitions::FarFlankColumns = {0, 4};
const int32 FFlankCellDefinitions::CenterRow = 2;
const TArray<int32> FFlankCellDefinitions::CenterColumns = {1, 2, 3};
bool FFlankCellDefinitions::IsAttackerFlankColumn(int32 Col)
{
	return AttackerFlankColumns.Contains(Col);
}
bool FFlankCellDefinitions::IsDefenderFlankColumn(int32 Col)
{
	return DefenderFlankColumns.Contains(Col);
}
bool FFlankCellDefinitions::IsCenterLineCell(int32 Row, int32 Col)
{
	return Row == CenterRow && CenterColumns.Contains(Col);
}
bool FFlankCellDefinitions::IsClosestFlankColumn(int32 Col)
{
	return ClosestFlankColumns.Contains(Col);
}
bool FFlankCellDefinitions::IsFarFlankColumn(int32 Col)
{
	return FarFlankColumns.Contains(Col);
}
bool FFlankCellDefinitions::IsEntranceFlankCell(int32 Row, int32 Col)
{
	return FTacCoordinates::IsFlankCell(Row, Col) && (Row == 1 || Row == 3);
}
bool FFlankCellDefinitions::IsRearFlankCell(int32 Row, int32 Col)
{
	return FTacCoordinates::IsFlankCell(Row, Col) && (Row == 0 || Row == 4);
}
int32 FFlankCellDefinitions::GetAdjacentNormalCol(int32 FlankCol)
{
	return FlankCol == 0 ? 1 : 3;
}
FTacCoordinates FFlankCellDefinitions::GetEntranceBlockedCell(int32 EntranceRow, int32 FlankCol)
{
	const int32 BlockedRow = (EntranceRow == 1) ? 0 : 4;
	return FTacCoordinates(BlockedRow, GetAdjacentNormalCol(FlankCol));
}

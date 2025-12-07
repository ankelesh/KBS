#include "GameplayTypes/FlankCellDefinitions.h"

// Static member definitions
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

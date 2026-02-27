#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/GridCoordinates.h"

struct KBS_API FFlankCellDefinitions
{
	static const TArray<int32> AttackerFlankColumns;
	static const TArray<int32> DefenderFlankColumns;
	static const TArray<int32> ClosestFlankColumns;
	static const TArray<int32> FarFlankColumns;
	static const int32 CenterRow;
	static const TArray<int32> CenterColumns;
	static bool IsAttackerFlankColumn(int32 Col);
	static bool IsDefenderFlankColumn(int32 Col);
	static bool IsCenterLineCell(int32 Row, int32 Col);
	static bool IsClosestFlankColumn(int32 Col);
	static bool IsFarFlankColumn(int32 Col);
	static bool IsEntranceFlankCell(int32 Row, int32 Col);
	static bool IsRearFlankCell(int32 Row, int32 Col);
	static int32 GetAdjacentNormalCol(int32 FlankCol);
	static FTacCoordinates GetEntranceBlockedCell(int32 EntranceRow, int32 FlankCol);
};
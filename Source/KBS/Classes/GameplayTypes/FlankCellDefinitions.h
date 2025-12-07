#pragma once

#include "CoreMinimal.h"

/**
 * Static definitions for flank cell positions and center line
 * Grid is 5x5 (0-4), with flanks at columns 0-1 (attacker) and 3-4 (defender)
 */
struct KBS_API FFlankCellDefinitions
{
	// Attacker flank columns (left side)
	static const TArray<int32> AttackerFlankColumns;

	// Defender flank columns (right side)
	static const TArray<int32> DefenderFlankColumns;

	// Closest-to-center flank columns
	static const TArray<int32> ClosestFlankColumns;

	// Far flank columns
	static const TArray<int32> FarFlankColumns;

	// Center row (where special flank access is granted)
	static const int32 CenterRow;

	// Center columns (the non-flank columns in center row: 1, 2, 3)
	static const TArray<int32> CenterColumns;

	// Helper: Check if column is attacker flank
	static bool IsAttackerFlankColumn(int32 Col);

	// Helper: Check if column is defender flank column
	static bool IsDefenderFlankColumn(int32 Col);

	// Helper: Check if cell is on center row (row 2) and in non-flank columns (1-3)
	static bool IsCenterLineCell(int32 Row, int32 Col);

	// Helper: Check if column is closest-to-center flank
	static bool IsClosestFlankColumn(int32 Col);

	// Helper: Check if column is far flank
	static bool IsFarFlankColumn(int32 Col);
};

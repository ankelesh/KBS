#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/TeamConstants.h"
#include "GameplayTypes/GridCoordinates.h"

struct KBS_API FFlankCellDefinitions
{
	static constexpr int32 CenterRow          = FGridConstants::CenterRow;
	static constexpr int32 FlankColLeft       = FGridConstants::ExcludedColLeft;
	static constexpr int32 FlankColRight      = FGridConstants::ExcludedColRight;
	static constexpr int32 EntranceLeftClosestCol = FGridConstants::ExcludedColLeft + 1;
	static constexpr int32 EntranceRightClosestCol = FGridConstants::ExcludedColRight - 1;
	static constexpr int32 FlankEntranceTop   = FGridConstants::CenterRow - 1;
	static constexpr int32 FlankEntranceBottom = FGridConstants::CenterRow + 1;
	static constexpr int32 FlankRearTop       = 0;
	static constexpr int32 FlankRearBottom    = FGridConstants::GridSize - 1;

	static const TArray<int32> CenterColumns;
	static const FTacCoordinates AttackerLeftEntranceCell;
	static const FTacCoordinates AttackerRightEntranceCell;
	static const FTacCoordinates DefenderLeftEntranceCell;
	static const FTacCoordinates DefenderRightEntranceCell;
	static const FTacCoordinates AttackerLeftRearCell;
	static const FTacCoordinates AttackerRightRearCell;
	static const FTacCoordinates DefenderLeftRearCell;
	static const FTacCoordinates DefenderRightRearCell;

	static bool IsAttackerFlank(FTacCoordinates Cell);
	static bool IsDefenderFlank(FTacCoordinates Cell);
	static bool IsCenterLineCell(FTacCoordinates Cell);
	static bool IsEntranceCell(FTacCoordinates Cell);
	static bool IsEntranceAvailable(FTacCoordinates Cell);
	static FTacCoordinates GetAvailableFlankCell(FTacCoordinates Cell, ETeamSide Team);
	static bool IsRearAvailable(FTacCoordinates Cell);
	static FTacCoordinates GetAdjacentNormalCell(FTacCoordinates Cell);
	static FTacCoordinates GetEntranceBlockedCell(FTacCoordinates Cell);
};

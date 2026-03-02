#include "GameplayTypes/FlankCellDefinitions.h"

const TArray<int32> FFlankCellDefinitions::CenterColumns = {1, 2, 3};
const FTacCoordinates FFlankCellDefinitions::AttackerLeftEntranceCell(FlankEntranceBottom, FlankColRight,
                                                                      ETacGridLayer::Ground);
const FTacCoordinates FFlankCellDefinitions::AttackerRightEntranceCell(FlankEntranceBottom, FlankColLeft,
                                                                       ETacGridLayer::Ground);
const FTacCoordinates FFlankCellDefinitions::DefenderLeftEntranceCell(FlankEntranceTop, FlankColLeft,
                                                                      ETacGridLayer::Ground);
const FTacCoordinates FFlankCellDefinitions::DefenderRightEntranceCell(FlankEntranceTop, FlankColRight,
                                                                       ETacGridLayer::Ground);
const FTacCoordinates
FFlankCellDefinitions::AttackerLeftRearCell(FlankRearBottom, FlankColRight, ETacGridLayer::Ground);
const FTacCoordinates
FFlankCellDefinitions::AttackerRightRearCell(FlankRearBottom, FlankColLeft, ETacGridLayer::Ground);
const FTacCoordinates FFlankCellDefinitions::DefenderLeftRearCell(FlankRearTop, FlankColLeft, ETacGridLayer::Ground);
const FTacCoordinates FFlankCellDefinitions::DefenderRightRearCell(FlankRearTop, FlankColRight, ETacGridLayer::Ground);

bool FFlankCellDefinitions::IsAttackerFlank(FTacCoordinates Cell)
{
	return (Cell.Col == FlankColLeft || Cell.Col == FlankColRight) && Cell.Row >= FlankEntranceBottom;
}

bool FFlankCellDefinitions::IsDefenderFlank(FTacCoordinates Cell)
{
	return (Cell.Col == FlankColLeft || Cell.Col == FlankColRight) && Cell.Row <= FlankEntranceTop;
}

bool FFlankCellDefinitions::IsCenterLineCell(FTacCoordinates Cell)
{
	switch (Cell.Col)
	{
	case 1:
	case 2:
	case 3:
		return true;
	default:
		return false;
	}
}

bool FFlankCellDefinitions::IsEntranceCell(FTacCoordinates Cell)
{
	return (Cell.Col == FlankColLeft || Cell.Col == FlankColRight) && (Cell.Row == FlankEntranceTop || Cell.Row ==
		FlankEntranceBottom);
}

bool FFlankCellDefinitions::IsEntranceAvailable(FTacCoordinates Cell)
{
	return Cell.Col == FlankColLeft + 1 || Cell.Col == FlankColRight - 1;
}

FTacCoordinates FFlankCellDefinitions::GetAvailableFlankCell(FTacCoordinates Cell, ETeamSide Team)
{
	if (Team == ETeamSide::Attacker)
	{
		if (IsRearAvailable(Cell))
		{
			return Cell.Col == AttackerLeftEntranceCell.Col ? AttackerLeftRearCell : AttackerRightRearCell;
		}
		if (IsEntranceAvailable(Cell))
		{
			// reversed since attacker relative is reversed
			return Cell.Col == EntranceRightClosestCol ? AttackerLeftEntranceCell : AttackerRightEntranceCell;
		}
	}
	if (Team == ETeamSide::Defender)
	{
		if (IsRearAvailable(Cell))
		{
			return Cell.Col == DefenderLeftEntranceCell.Col ? DefenderLeftRearCell : DefenderRightRearCell;
		}
		if (IsEntranceAvailable(Cell))
		{
			return Cell.Col == EntranceLeftClosestCol ? DefenderLeftEntranceCell : DefenderRightEntranceCell;
		}
	}
	return FTacCoordinates::Invalid();
}

bool FFlankCellDefinitions::IsRearAvailable(FTacCoordinates Cell)
{
	return (Cell.Col == FlankColLeft || Cell.Col == FlankColRight)
		&& (Cell.Row == FlankEntranceTop || Cell.Row == FlankEntranceBottom);
}

FTacCoordinates FFlankCellDefinitions::GetAdjacentNormalCell(FTacCoordinates Cell)
{
	const int32 NormalCol = (Cell.Col == FlankColLeft) ? FlankColLeft + 1 : FlankColRight - 1;
	return FTacCoordinates(Cell.Row, NormalCol, Cell.Layer);
}

FTacCoordinates FFlankCellDefinitions::GetEntranceBlockedCell(FTacCoordinates Cell)
{
	if (IsEntranceCell(Cell))
	{
		const int32 RearRow = (Cell.Row == FlankEntranceTop) ? FlankRearTop : FlankRearBottom;
		return GetAdjacentNormalCell(FTacCoordinates(RearRow, Cell.Col, Cell.Layer));
	}
	return FTacCoordinates::Invalid();
}

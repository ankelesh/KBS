#include "GameMechanics/Units/LargeUnit.h"

FIntPoint ALargeUnit::GetSecondaryCell(int32 PrimaryRow, int32 PrimaryCol, bool bIsHorizontal, ETeamSide Team)
{
	if (bIsHorizontal)
	{
		// Horizontal: Same row, column extends toward center
		// Left flank (col 0) -> secondary at col+1
		// Right flank (col 4) -> secondary at col-1
		int32 SecondaryCol = (PrimaryCol == 0) ? PrimaryCol + 1 : PrimaryCol - 1;
		return FIntPoint(SecondaryCol, PrimaryRow);
	}
	else
	{
		// Vertical: Same column, row direction depends on team
		// Attackers: Row+1 (forward)
		// Defenders: Row-1 (backward)
		int32 SecondaryRow = (Team == ETeamSide::Attacker) ? PrimaryRow + 1 : PrimaryRow - 1;
		return FIntPoint(PrimaryCol, SecondaryRow);
	}
}

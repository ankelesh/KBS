#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Units/Unit.h"

namespace KbsAlgo
{
	template <template<typename...> class TContainer, typename P, typename D>
	TContainer<FTacCoordinates> CopyAdjacentIf(FTacCoordinates Origin, D* DataManager, P Predicate)
	{
		TContainer<FTacCoordinates> FoundCells;
		for (const FIntPoint& Offset : FGridConstants::AllAdjacentOffsets)
		{
			FTacCoordinates TargetCoords(Origin.Row + Offset.X, Origin.Col + Offset.Y, Origin.Layer);
			if (!TargetCoords.IsValidCell())
			{
				continue;
			}
			AUnit* TargetUnit = DataManager->GetUnit(TargetCoords);
			if (!TargetUnit)
			{
				continue;
			}
			if (Predicate(TargetUnit))
			{
				checkf(!TargetUnit->IsDead(), TEXT("Dead unit found not in corpse stack!"));
				FoundCells.Add(TargetUnit->GetGridMetadata().Coords);
			}
		}
		return FoundCells;
	}

} // namespace KbsAlgo

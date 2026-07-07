#pragma once
#include "CoreMinimal.h"

struct FTacLogMoveStep;
class UGridDataManager;
class UMovePresentationAction;
class AUnit;

namespace TacticalLogConverters
{
	// Converts a single move hop into a fully configured move action.
	// UnitLookup must contain Step.UnitId - a move step referencing an untracked unit is a broken log contract.
	UMovePresentationAction* ConvertMoveStep(const FTacLogMoveStep& Step, const TMap<FGuid, AUnit*>& UnitLookup, UGridDataManager* GridDataManager);
}

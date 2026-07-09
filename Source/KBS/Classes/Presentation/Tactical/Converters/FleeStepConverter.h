#pragma once
#include "CoreMinimal.h"

struct FTacLogFleeStep;
class UGridDataManager;
class UMovePresentationAction;
class AUnit;

namespace TacticalLogConverters
{
	// Converts a flee marker into a rotation-only move (zero-length segment) facing the unit's field side.
	// UnitLookup must contain Step.UnitId - a flee step referencing an untracked unit is a broken log contract.
	UMovePresentationAction* ConvertFleeStep(const FTacLogFleeStep& Step, const TMap<FGuid, AUnit*>& UnitLookup, UGridDataManager* GridDataManager);
}

#pragma once
#include "CoreMinimal.h"
#include "Presentation/PresentationBuildContext.h"

struct FTacLogFleeStep;
class UMovePresentationAction;

namespace TacticalLogConverters
{
	// Converts a flee marker into a rotation-only move (zero-length segment) facing the unit's field side.
	// Step.UnitId must be present in Context.UnitLookup - a flee step referencing an untracked unit
	// is a broken log contract (unit is present in UnitLookup).
	UMovePresentationAction* ConvertFleeStep(const FTacLogFleeStep& Step, const FPresentationBuildContext& Context);
}

#pragma once
#include "CoreMinimal.h"
#include "Presentation/PresentationBuildContext.h"

struct FTacLogMoveStep;
class UMovePresentationAction;

namespace TacticalLogConverters
{
	// Converts a single move hop into a fully configured move action.
	// Step.UnitId must be present in Context.UnitLookup - a move step referencing an untracked unit
	// is a broken log contract (unit is present in UnitLookup).
	UMovePresentationAction* ConvertMoveStep(const FTacLogMoveStep& Step, const FPresentationBuildContext& Context);
}

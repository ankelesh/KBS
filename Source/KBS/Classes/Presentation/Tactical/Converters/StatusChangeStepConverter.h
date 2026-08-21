#pragma once
#include "CoreMinimal.h"
#include "Presentation/PresentationBuildContext.h"

struct FTacLogStatusChangeStep;
class UFloatingTextPresentationAction;

namespace TacticalLogConverters
{
	// Converts a status change into a floating label with the status name.
	// Step.UnitId must be present in Context.UnitLookup - a status step referencing an untracked
	// unit is a broken log contract (unit is present in UnitLookup).
	UFloatingTextPresentationAction* ConvertStatusChangeStep(const FTacLogStatusChangeStep& Step, const FPresentationBuildContext& Context);
}

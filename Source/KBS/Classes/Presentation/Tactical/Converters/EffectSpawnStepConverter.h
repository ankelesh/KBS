#pragma once
#include "CoreMinimal.h"
#include "Presentation/PresentationBuildContext.h"

struct FTacLogEffectSpawnStep;
class UFloatingTextPresentationAction;

namespace TacticalLogConverters
{
	// Converts an effect spawn into a floating label with the effect's name, colored by polarity.
	// Step.TargetUnitId must be present in Context.UnitLookup - an effect step referencing an
	// untracked unit is a broken log contract (unit is present in UnitLookup).
	UFloatingTextPresentationAction* ConvertEffectSpawnStep(const FTacLogEffectSpawnStep& Step, const FPresentationBuildContext& Context);
}

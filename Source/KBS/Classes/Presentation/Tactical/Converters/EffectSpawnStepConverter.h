#pragma once
#include "CoreMinimal.h"

struct FTacLogEffectSpawnStep;
class UFloatingTextPresentationAction;
class AUnit;

namespace TacticalLogConverters
{
	// Converts an effect spawn into a floating label with the effect's name, colored by polarity.
	// UnitLookup must contain Step.TargetUnitId - an effect step referencing an untracked unit is a broken log contract.
	UFloatingTextPresentationAction* ConvertEffectSpawnStep(const FTacLogEffectSpawnStep& Step, const TMap<FGuid, AUnit*>& UnitLookup);
}

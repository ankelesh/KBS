#pragma once
#include "CoreMinimal.h"

struct FTacLogStatusChangeStep;
class UFloatingTextPresentationAction;
class AUnit;

namespace TacticalLogConverters
{
	// Converts a status change into a floating label with the status name.
	// UnitLookup must contain Step.UnitId - a status step referencing an untracked unit is a broken log contract.
	UFloatingTextPresentationAction* ConvertStatusChangeStep(const FTacLogStatusChangeStep& Step, const TMap<FGuid, AUnit*>& UnitLookup);
}

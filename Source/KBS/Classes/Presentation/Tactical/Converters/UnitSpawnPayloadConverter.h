#pragma once
#include "CoreMinimal.h"
#include "Presentation/PresentationBuildContext.h"

struct FUnitSpawnPayload;
class UFloatingTextPresentationAction;

namespace TacticalLogConverters
{
	// Converts a unit spawn into a floating "Spawned" label.
	// Payload.SpawnedUnitId must be present in Context.UnitLookup - a spawn payload referencing
	// an untracked unit is a broken log contract (unit is present in UnitLookup).
	UFloatingTextPresentationAction* ConvertUnitSpawnPayload(const FUnitSpawnPayload& Payload, const FPresentationBuildContext& Context);
}

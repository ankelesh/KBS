#pragma once
#include "CoreMinimal.h"

struct FUnitSpawnPayload;
class UFloatingTextPresentationAction;
class AUnit;

namespace TacticalLogConverters
{
	// Converts a unit spawn into a floating "Spawned" label.
	// UnitLookup must contain Payload.SpawnedUnitId - a spawn payload referencing an untracked unit is a broken log contract.
	UFloatingTextPresentationAction* ConvertUnitSpawnPayload(const FUnitSpawnPayload& Payload, const TMap<FGuid, AUnit*>& UnitLookup);
}

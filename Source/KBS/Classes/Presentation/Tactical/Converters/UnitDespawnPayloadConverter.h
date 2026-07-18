#pragma once
#include "CoreMinimal.h"

struct FUnitDespawnPayload;
class UDespawnPresentationAction;
class AUnit;

namespace TacticalLogConverters
{
	// Converts a unit despawn into a UDespawnPresentationAction, whose OnCleanup finalizes the despawn.
	// UnitLookup must contain Payload.UnitId (via EUnitQuerySource::PendingDespawn) - a despawn payload
	// referencing an untracked unit is a broken log contract.
	UDespawnPresentationAction* ConvertUnitDespawnPayload(const FUnitDespawnPayload& Payload, const TMap<FGuid, AUnit*>& UnitLookup);
}

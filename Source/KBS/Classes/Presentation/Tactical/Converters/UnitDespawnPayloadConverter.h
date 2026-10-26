#pragma once
#include "CoreMinimal.h"
#include "Presentation/PresentationBuildContext.h"

struct FUnitDespawnPayload;
class UDespawnPresentationAction;

namespace TacticalLogConverters
{
	// Converts a unit despawn into a UDespawnPresentationAction, whose OnCleanup finalizes the despawn.
	// Payload.UnitId must be present in Context.UnitLookup (via PendingDespawn) - a despawn payload
	// referencing an untracked unit is a broken log contract (unit is present in UnitLookup).
	UDespawnPresentationAction* ConvertUnitDespawnPayload(const FUnitDespawnPayload& Payload, const FPresentationBuildContext& Context);
}

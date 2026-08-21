#pragma once
#include "CoreMinimal.h"
#include "Presentation/PresentationBuildContext.h"

struct FUnitMoveOffFieldPayload;
class UMovePresentationAction;

namespace TacticalLogConverters
{
	// Converts a unit's field exit into a move action running the unit off-field toward its own baseline.
	// Payload.UnitId must be present in Context.UnitLookup - a move-off-field payload referencing
	// an untracked unit is a broken log contract (unit is present in UnitLookup).
	UMovePresentationAction* ConvertUnitMoveOffFieldPayload(const FUnitMoveOffFieldPayload& Payload, const FPresentationBuildContext& Context);
}

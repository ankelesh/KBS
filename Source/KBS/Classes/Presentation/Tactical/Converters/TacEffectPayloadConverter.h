#pragma once
#include "CoreMinimal.h"
#include "Presentation/PresentationBuildContext.h"

struct FTacEffectPayload;
class UVfxPresentationAction;

namespace TacticalLogConverters
{
	// Converts an effect-trigger event into its Niagara VFX. Returns nullptr if the effect has no VFX.
	// Payload.OwnerUnitId must be present in Context.UnitLookup - an effect payload referencing
	// an untracked unit is a broken log contract (unit is present in UnitLookup).
	UVfxPresentationAction* ConvertTacEffectPayload(const FTacEffectPayload& Payload, const FPresentationBuildContext& Context);
}

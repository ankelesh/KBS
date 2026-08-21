#pragma once
#include "CoreMinimal.h"
#include "Presentation/PresentationBuildContext.h"

struct FEffectEndPayload;
class UFloatingTextPresentationAction;

namespace TacticalLogConverters
{
	// Converts an effect expiry into a floating "<EffectName> ended" label.
	// Payload.OwnerUnitId must be present in Context.UnitLookup - an effect end payload referencing
	// an untracked unit is a broken log contract (unit is present in UnitLookup).
	UFloatingTextPresentationAction* ConvertEffectEndPayload(const FEffectEndPayload& Payload, const FPresentationBuildContext& Context);
}

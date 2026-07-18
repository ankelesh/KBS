#pragma once
#include "CoreMinimal.h"

struct FEffectEndPayload;
class UFloatingTextPresentationAction;
class AUnit;

namespace TacticalLogConverters
{
	// Converts an effect expiry into a floating "<EffectName> ended" label.
	// UnitLookup must contain Payload.OwnerUnitId - an effect end payload referencing an untracked unit is a broken log contract.
	UFloatingTextPresentationAction* ConvertEffectEndPayload(const FEffectEndPayload& Payload, const TMap<FGuid, AUnit*>& UnitLookup);
}

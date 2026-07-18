#pragma once
#include "CoreMinimal.h"

struct FTacEffectPayload;
class UVfxPresentationAction;
class AUnit;

namespace TacticalLogConverters
{
	// Converts an effect-trigger event into its Niagara VFX. Returns nullptr if the effect has no VFX
	// (e.g. an innate status).
	// UnitLookup must contain Payload.OwnerUnitId - an effect payload referencing an untracked unit is a broken log contract.
	UVfxPresentationAction* ConvertTacEffectPayload(const FTacEffectPayload& Payload, const TMap<FGuid, AUnit*>& UnitLookup);
}

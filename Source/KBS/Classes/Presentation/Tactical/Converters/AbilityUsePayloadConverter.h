#pragma once
#include "CoreMinimal.h"
#include "Presentation/PresentationBuildContext.h"

struct FAbilityUsePayload;
class UFloatingTextPresentationAction;

namespace TacticalLogConverters
{
	// Converts an ability use into a floating "<AbilityName>" label above the unit that used it.
	// Returns nullptr when there is nothing worth announcing: plain movement/wait, or no backing asset/name.
	// Instigator must be present in Context.UnitLookup.
	UFloatingTextPresentationAction* ConvertAbilityUsePayload(const FAbilityUsePayload& Payload, const FGuid& InstigatorId,
	                                                          const FPresentationBuildContext& Context);
}

#pragma once
#include "CoreMinimal.h"
#include "Presentation/PresentationBuildContext.h"

struct FTacLogInstantChangeStep;
class UPresentationSequenceAction;

namespace TacticalLogConverters
{
	// Appends one floating label for the heal (if any) and one per granted ward.
	// Step.UnitId must be present in Context.UnitLookup.
	void ConvertInstantChangeStep(const FTacLogInstantChangeStep& Step, const FPresentationBuildContext& Context,
	                              TArray<TObjectPtr<UPresentationSequenceAction>>& OutActions);
}

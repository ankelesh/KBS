#pragma once
#include "CoreMinimal.h"
#include "Presentation/PresentationBuildContext.h"

struct FTacLogCombatStep;
class UPresentationSequenceAction;

namespace TacticalLogConverters
{
	// Converts a combat step into: attacker swing -> delivery -> per-hit damage labels ->
	// AoE reaction chains -> attacker flourish (if all targets killed).
	// AttackerId absent from UnitLookup is legal (DOT ticks) - swing and flourish are skipped.
	// Every TargetId must be present in UnitLookup - a combat step referencing an untracked unit
	// is a broken log contract (unit is present in UnitLookup).
	void ConvertCombatStep(const FTacLogCombatStep& Step, const FPresentationBuildContext& Context,
	                       TArray<TObjectPtr<UPresentationSequenceAction>>& OutActions);
}

#pragma once
#include "CoreMinimal.h"

struct FTacLogCombatStep;
class UPresentationSequenceAction;
class AUnit;

namespace TacticalLogConverters
{
	// Converts a combat step into attacker swing + per-hit damage-number actions, plus one
	// UAoEReactionPresentationAction that plays every hit target's reaction/death chain
	// simultaneously (so AOE hits react together instead of one at a time).
	// UnitLookup must contain every hit's TargetId - a combat step referencing an untracked unit
	// is a broken log contract. AttackerId may be invalid/untracked (e.g. DOT ticks have no attacker
	// unit) - the swing action is simply skipped in that case.
	void ConvertCombatStep(const FTacLogCombatStep& Step, const TMap<FGuid, AUnit*>& UnitLookup,
	                       TArray<TObjectPtr<UPresentationSequenceAction>>& OutActions);
}

#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/Spells/OffensiveSpellAbility.h"
#include "MindDominanceAbility.generated.h"

/**
 * Single-target spell whose damage applies only to a target that owns a spellbook; the descriptor's effects
 * (e.g. paralysis) still apply to any target. Needs scaling enabled: the scaled magnitude is what gets zeroed.
 */
UCLASS()
class KBS_API UMindDominanceAbility : public UOffensiveSpellAbility
{
	GENERATED_BODY()

protected:
	virtual void AdjustDescriptorForTargets(const FResolvedTargets& Targets) const override;
};

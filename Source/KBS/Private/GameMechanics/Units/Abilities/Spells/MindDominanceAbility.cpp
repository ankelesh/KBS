#include "GameMechanics/Units/Abilities/Spells/MindDominanceAbility.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/CombatTypes.h"

void UMindDominanceAbility::AdjustDescriptorForTargets(const FResolvedTargets& Targets) const
{
	if (!Targets.ClickedTarget) return;

	if (Targets.ClickedTarget->GetAbilityInventory()->GetSpellbookAbilities().IsEmpty())
	{
		EmbeddedDescriptor->SetMagnitudeBase(0);
	}
}

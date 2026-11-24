#include "GameMechanics/Units/Abilities/Empire/QuickStrikeAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameplayTypes/Tags/Tactical/AbilityTags.h"

FGameplayTagContainer UQuickStrikeAbility::BuildTags() const
{
	FGameplayTagContainer Tags = Super::BuildTags();
	Tags.AddTag(TAG_ABILITY_DEFEND);
	return Tags;
}

UCombatDescriptor* UQuickStrikeAbility::ResolveAttackDescriptor(UCombatDescriptor* WeaponDescriptor) const
{
	// IsMutable() reports bIsImmutable: magnitude modifiers are silently ignored on such descriptors.
	checkf(!WeaponDescriptor->IsMutable(), TEXT("%s: weapon descriptor is immutable, cannot scale damage"),
	       *GetDebugString());

	// The copy carries the weapon's current modifiers, so buffs on the weapon still apply to the strike.
	UQuickStrikeAbility* MutableThis = const_cast<UQuickStrikeAbility*>(this);
	MutableThis->ScaledDescriptor = DuplicateObject<UCombatDescriptor>(WeaponDescriptor, MutableThis);
	ScaledDescriptor->ModifyMagnitude(DamagePercent - 100, AbilityId, false);
	return ScaledDescriptor;
}

void UQuickStrikeAbility::OnAttackResolved(const UCombatDescriptor* UsedDescriptor,
                                           TArray<TInstancedStruct<FTacLogStepBase>>& OutSteps)
{
	if (Owner->IsDead())
		return;

	Owner->GetStats().Status.SetDefending();
	OutSteps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogStatusChangeStep>(
		FTacLogStatusChangeStep::Make(Owner->GetUnitID(), EUnitStatus::Defending, true)));
}

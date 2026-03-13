#pragma once
#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "NativeGameplayTags.h"
#include "GameplayTagsManager.h"
#include "GameplayTypes/CombatDescriptorTypes.h"
#include "GameplayTypes/EffectTypes.h"
class UBattleEffect;

// Class tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_AUTOATTACK)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_MOVEMENT)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_WAIT)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_DEFEND)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_FLEE)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_SPELL)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_SPELL_SUMMON)

// Intent tags — deduced from CombatDescriptor
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_ATTACK)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_HEAL)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_BUFF)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_DEBUFF)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_EFFECT)

namespace AbilityTagUtils
{
	// Pure policy-to-tag mapping, no class context. Returns EmptyTag for None/unknown.
	inline FGameplayTag TagFromPolicy(EMagnitudePolicy Policy)
	{
		switch (Policy)
		{
		case EMagnitudePolicy::Damage: return TAG_ABILITY_ATTACK;
		case EMagnitudePolicy::Heal:   return TAG_ABILITY_HEAL;
		default:                       return FGameplayTag::EmptyTag;
		}
	}

	// Adds TAG_ABILITY_EFFECT for any effect; additionally TAG_ABILITY_BUFF/DEBUFF based on polarity.
	inline void AddEffectTags(FGameplayTagContainer& Tags, const TArray<UBattleEffect*>& Effects)
	{
		for (const UBattleEffect* Effect : Effects)
		{
			Tags.AddTag(TAG_ABILITY_EFFECT);
			switch (Effect->GetPolarity())
			{
			case EEffectPolarity::Positive: Tags.AddTag(TAG_ABILITY_BUFF);   break;
			case EEffectPolarity::Negative: Tags.AddTag(TAG_ABILITY_DEBUFF); break;
			default: break;
			}
		}
	}
}

#pragma once
#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "NativeGameplayTags.h"
#include "GameplayTagsManager.h"
#include "GameplayTypes/CombatDescriptorTypes.h"

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
	// Pure intent-to-tag mapping, no class context. Returns EmptyTag for Auto/unknown.
	inline FGameplayTag TagFromIntent(ECombatIntent Intent)
	{
		switch (Intent)
		{
		case ECombatIntent::Attack:            return TAG_ABILITY_ATTACK;
		case ECombatIntent::Heal:              return TAG_ABILITY_HEAL;
		case ECombatIntent::BuffApplication:   return TAG_ABILITY_BUFF;
		case ECombatIntent::DebuffApplication: return TAG_ABILITY_DEBUFF;
		case ECombatIntent::EffectApplication: return TAG_ABILITY_EFFECT;
		default:                               return FGameplayTag::EmptyTag;
		}
	}
}

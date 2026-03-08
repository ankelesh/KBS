#include "GameplayTypes/AbilityTypes.h"

bool FAbilityContext::CanAct(const bool bIsCurrentAbility) const
{
	switch (TurnState)
	{
	case EAbilityTurnReleasePolicy::Free:
		return true;
	case EAbilityTurnReleasePolicy::Locked:
		return bIsCurrentAbility;
	case EAbilityTurnReleasePolicy::Released:
	case EAbilityTurnReleasePolicy::Conditional:
	default:
		return false;
	}
}

bool FAbilityContext::CanConditionalAct(const bool bIsCurrentAbility, const FGameplayTag& Tag,
                                        const bool bIsPersistent) const
{
	switch (TurnState)
	{
	case EAbilityTurnReleasePolicy::Free:
		return true;
	case EAbilityTurnReleasePolicy::Locked:
		return bIsCurrentAbility;
	case EAbilityTurnReleasePolicy::Released:
	default:
		return false;
	case EAbilityTurnReleasePolicy::Conditional:
		return HasTag(Tag, bIsPersistent);
	}
}

bool FAbilityContext::HasTurnTag(const FGameplayTag& Tag) const
{
	return TurnTagContext.HasTag(Tag);
}

bool FAbilityContext::HasPersistentTurnTag(const FGameplayTag& Tag) const
{
	return PersistentTurnTagContext.HasTag(Tag);
}

bool FAbilityContext::HasTag(const FGameplayTag& Tag, bool bIsPersistent) const
{
	return bIsPersistent ? HasPersistentTurnTag(Tag) : HasTurnTag(Tag);
}

bool FAbilityContext::HasTagAnywhere(const FGameplayTag& Tag) const
{
	return TurnTagContext.HasTag(Tag) || PersistentTurnTagContext.HasTag(Tag);
}

void FAbilityContext::AddTag(const FGameplayTag& Tag, bool bIsPersistent)
{
	if (bIsPersistent)
		PersistentTurnTagContext.AddTag(Tag);
	else
		TurnTagContext.AddTag(Tag);
}

void FAbilityContext::RemoveTag(const FGameplayTag& Tag, bool bIsPersistent)
{
	if (bIsPersistent)
		PersistentTurnTagContext.RemoveTag(Tag);
	else
		TurnTagContext.RemoveTag(Tag);
}

void FAbilityContext::Lock()
{
	StateBeforeLock = TurnState;
	TurnState = EAbilityTurnReleasePolicy::Locked;
}

void FAbilityContext::Unlock()
{
	TurnState = StateBeforeLock;
}

void FAbilityContext::ClearTurnData()
{
	TurnTagContext.Reset();
	TurnOnlyStorage.Empty();
}

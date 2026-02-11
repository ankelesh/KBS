#include "GameMechanics/Units/Stats/UnitStatusContainer.h"

bool FUnitStatusContainer::AddStatus(EUnitStatus Status, const FGuid& EffectId)
{
	switch (Status)
	{
	case EUnitStatus::TurnBlocked:
		{
			const bool bWasInactive = TurnBlockedModifiers.Num() == 0;
			TurnBlockedModifiers.Add(EffectId);
			return bWasInactive;
		}
	case EUnitStatus::Pinned:
		{
			const bool bWasInactive = PinnedModifiers.Num() == 0;
			PinnedModifiers.Add(EffectId);
			return bWasInactive;
		}
	case EUnitStatus::Silenced:
		{
			const bool bWasInactive = SilencedModifiers.Num() == 0;
			SilencedModifiers.Add(EffectId);
			return bWasInactive;
		}
	case EUnitStatus::Disoriented:
		{
			const bool bWasInactive = DisorientedModifiers.Num() == 0;
			DisorientedModifiers.Add(EffectId);
			return bWasInactive;
		}
	case EUnitStatus::Focused:
		{
			const bool bWasInactive = !bFocused;
			bFocused = true;
			return bWasInactive;
		}
	case EUnitStatus::Fleeing:
		{
			const bool bWasInactive = !bFleeing;
			bFleeing = true;
			return bWasInactive;
		}
	case EUnitStatus::Channeling:
		{
			const bool bWasInactive = !bChanneling;
			bChanneling = true;
			return bWasInactive;
		}
	default:
		return false;
	}
}

bool FUnitStatusContainer::RemoveStatus(EUnitStatus Status, const FGuid& EffectId)
{
	switch (Status)
	{
	case EUnitStatus::TurnBlocked:
		{
			TurnBlockedModifiers.Remove(EffectId);
			return TurnBlockedModifiers.Num() == 0;
		}
	case EUnitStatus::Pinned:
		{
			PinnedModifiers.Remove(EffectId);
			return PinnedModifiers.Num() == 0;
		}
	case EUnitStatus::Silenced:
		{
			SilencedModifiers.Remove(EffectId);
			return SilencedModifiers.Num() == 0;
		}
	case EUnitStatus::Disoriented:
		{
			DisorientedModifiers.Remove(EffectId);
			return DisorientedModifiers.Num() == 0;
		}
	case EUnitStatus::Focused:
		{
			const bool bWasActive = bFocused;
			bFocused = false;
			return bWasActive;
		}
	case EUnitStatus::Fleeing:
		{
			const bool bWasActive = bFleeing;
			bFleeing = false;
			return bWasActive;
		}
	case EUnitStatus::Channeling:
		{
			const bool bWasActive = bChanneling;
			bChanneling = false;
			return bWasActive;
		}
	default:
		return false;
	}
}

void FUnitStatusContainer::ClearStatus(EUnitStatus Status)
{
	switch (Status)
	{
	case EUnitStatus::TurnBlocked:
		TurnBlockedModifiers.Empty();
		break;
	case EUnitStatus::Pinned:
		PinnedModifiers.Empty();
		break;
	case EUnitStatus::Silenced:
		SilencedModifiers.Empty();
		break;
	case EUnitStatus::Disoriented:
		DisorientedModifiers.Empty();
		break;
	case EUnitStatus::Focused:
		bFocused = false;
		break;
	case EUnitStatus::Fleeing:
		bFleeing = false;
		break;
	case EUnitStatus::Channeling:
		bChanneling = false;
		break;
	}
}

void FUnitStatusContainer::ClearAll()
{
	TurnBlockedModifiers.Empty();
	PinnedModifiers.Empty();
	SilencedModifiers.Empty();
	DisorientedModifiers.Empty();
	bFocused = false;
	bFleeing = false;
	bChanneling = false;
}

bool FUnitStatusContainer::CanAct() const
{
	return TurnBlockedModifiers.Num() == 0;
}

bool FUnitStatusContainer::CanMove() const
{
	return CanAct() && PinnedModifiers.Num() == 0;
}

bool FUnitStatusContainer::CanUseSpellbook() const
{
	return CanAct() && SilencedModifiers.Num() == 0;
}

bool FUnitStatusContainer::CanUseNonBasicAbilities() const
{
	return CanAct() && DisorientedModifiers.Num() == 0;
}

bool FUnitStatusContainer::IsStatusActive(EUnitStatus Status) const
{
	switch (Status)
	{
	case EUnitStatus::TurnBlocked:
		return TurnBlockedModifiers.Num() > 0;
	case EUnitStatus::Pinned:
		return PinnedModifiers.Num() > 0;
	case EUnitStatus::Silenced:
		return SilencedModifiers.Num() > 0;
	case EUnitStatus::Disoriented:
		return DisorientedModifiers.Num() > 0;
	case EUnitStatus::Focused:
		return bFocused;
	case EUnitStatus::Fleeing:
		return bFleeing;
	case EUnitStatus::Channeling:
		return bChanneling;
	default:
		return false;
	}
}

#include "GameMechanics/Units/EffectManagerComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Unit.h"

UEffectManagerComponent::UEffectManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEffectManagerComponent::AddEffect(UBattleEffect* Effect)
{
	if (Effect)
	{
		ActiveEffects.Add(Effect);
		Effect->OnApplied(GetOwnerUnit());
	}
}

void UEffectManagerComponent::RemoveEffect(UBattleEffect* Effect)
{
	if (Effect)
	{
		Effect->OnRemoved(GetOwnerUnit());
		ActiveEffects.Remove(Effect);
	}
}

void UEffectManagerComponent::ClearAllEffects()
{
	AUnit* Owner = GetOwnerUnit();
	for (UBattleEffect* Effect : ActiveEffects)
	{
		if (Effect)
		{
			Effect->OnRemoved(Owner);
		}
	}
	ActiveEffects.Empty();
}

void UEffectManagerComponent::BroadcastTurnStart()
{
	AUnit* Owner = GetOwnerUnit();
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnTurnStart(Owner);
			Effect->DecrementTurns();

			if (Effect->GetRemainingTurns() <= 0)
			{
				RemoveEffect(Effect);
			}
		}
	}
}

void UEffectManagerComponent::BroadcastTurnEnd()
{
	AUnit* Owner = GetOwnerUnit();
	for (UBattleEffect* Effect : ActiveEffects)
	{
		if (Effect)
		{
			Effect->OnTurnEnd(Owner);
		}
	}
}

void UEffectManagerComponent::BroadcastAttacked(AUnit* Attacker)
{
	AUnit* Owner = GetOwnerUnit();
	for (UBattleEffect* Effect : ActiveEffects)
	{
		if (Effect)
		{
			Effect->OnUnitAttacked(Owner, Attacker);
		}
	}
}

void UEffectManagerComponent::BroadcastAttacks(AUnit* Target)
{
	AUnit* Owner = GetOwnerUnit();
	for (UBattleEffect* Effect : ActiveEffects)
	{
		if (Effect)
		{
			Effect->OnUnitAttacks(Owner, Target);
		}
	}
}

void UEffectManagerComponent::BroadcastMoved()
{
	AUnit* Owner = GetOwnerUnit();
	for (UBattleEffect* Effect : ActiveEffects)
	{
		if (Effect)
		{
			Effect->OnUnitMoved(Owner);
		}
	}
}

void UEffectManagerComponent::BroadcastDied()
{
	AUnit* Owner = GetOwnerUnit();
	for (UBattleEffect* Effect : ActiveEffects)
	{
		if (Effect)
		{
			Effect->OnUnitDied(Owner);
		}
	}
}

AUnit* UEffectManagerComponent::GetOwnerUnit() const
{
	return Cast<AUnit>(GetOwner());
}

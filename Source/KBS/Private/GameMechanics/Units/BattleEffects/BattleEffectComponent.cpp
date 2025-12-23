#include "GameMechanics/Units/BattleEffects/BattleEffectComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Unit.h"
UBattleEffectComponent::UBattleEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UBattleEffectComponent::AddEffect(UBattleEffect* Effect)
{
	if (Effect)
	{
		ActiveEffects.Add(Effect);
		Effect->OnApplied(GetOwnerUnit());
	}
}
void UBattleEffectComponent::RemoveEffect(UBattleEffect* Effect)
{
	if (Effect)
	{
		Effect->OnRemoved(GetOwnerUnit());
		ActiveEffects.Remove(Effect);
	}
}
void UBattleEffectComponent::ClearAllEffects()
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
void UBattleEffectComponent::BroadcastTurnStart()
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
void UBattleEffectComponent::BroadcastTurnEnd()
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
void UBattleEffectComponent::BroadcastAttacked(AUnit* Attacker)
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
void UBattleEffectComponent::BroadcastAttacks(AUnit* Target)
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
void UBattleEffectComponent::BroadcastMoved()
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
void UBattleEffectComponent::BroadcastDied()
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
AUnit* UBattleEffectComponent::GetOwnerUnit() const
{
	return Cast<AUnit>(GetOwner());
}

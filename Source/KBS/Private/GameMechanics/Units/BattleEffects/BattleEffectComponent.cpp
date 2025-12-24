#include "GameMechanics/Units/BattleEffects/BattleEffectComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Unit.h"
UBattleEffectComponent::UBattleEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UBattleEffectComponent::AddEffect(UBattleEffect* Effect)
{
	if (!Effect)
	{
		UE_LOG(LogTemp, Warning, TEXT("BattleEffectComponent::AddEffect - Null effect passed"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("BattleEffectComponent::AddEffect - Adding effect %s to %s"),
		*Effect->GetClass()->GetName(),
		GetOwnerUnit() ? *GetOwnerUnit()->GetName() : TEXT("NULL_OWNER"));

	FName StackingId = Effect->GetStackingId();
	if (StackingId != NAME_None)
	{
		for (UBattleEffect* ExistingEffect : ActiveEffects)
		{
			if (ExistingEffect && ExistingEffect->GetStackingId() == StackingId)
			{
				if (ExistingEffect->HandleReapply(Effect, GetOwnerUnit()))
				{
					UE_LOG(LogTemp, Log, TEXT("BattleEffectComponent::AddEffect - Effect reapplied via stacking"));
					return;
				}
			}
		}
	}
	ActiveEffects.Add(Effect);
	UE_LOG(LogTemp, Log, TEXT("BattleEffectComponent::AddEffect - Effect added to ActiveEffects, calling OnApplied"));
	Effect->OnApplied(GetOwnerUnit());
	UE_LOG(LogTemp, Log, TEXT("BattleEffectComponent::AddEffect - OnApplied called, total effects: %d"), ActiveEffects.Num());
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
			if (Effect->IsExpired())
			{
				RemoveEffect(Effect);
			}
		}
	}
}
void UBattleEffectComponent::BroadcastTurnEnd()
{
	AUnit* Owner = GetOwnerUnit();
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnTurnEnd(Owner);
			if (Effect->IsExpired())
			{
				RemoveEffect(Effect);
			}
		}
	}
}
void UBattleEffectComponent::BroadcastAttacked(AUnit* Attacker)
{
	AUnit* Owner = GetOwnerUnit();
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnUnitAttacked(Owner, Attacker);
			if (Effect->IsExpired())
			{
				RemoveEffect(Effect);
			}
		}
	}
}
void UBattleEffectComponent::BroadcastAttacks(AUnit* Target)
{
	AUnit* Owner = GetOwnerUnit();
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnUnitAttacks(Owner, Target);
			if (Effect->IsExpired())
			{
				RemoveEffect(Effect);
			}
		}
	}
}
void UBattleEffectComponent::BroadcastMoved()
{
	AUnit* Owner = GetOwnerUnit();
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnUnitMoved(Owner);
			if (Effect->IsExpired())
			{
				RemoveEffect(Effect);
			}
		}
	}
}
void UBattleEffectComponent::BroadcastDied()
{
	AUnit* Owner = GetOwnerUnit();
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnUnitDied(Owner);
			if (Effect->IsExpired())
			{
				RemoveEffect(Effect);
			}
		}
	}
}
AUnit* UBattleEffectComponent::GetOwnerUnit() const
{
	return Cast<AUnit>(GetOwner());
}

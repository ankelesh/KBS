#include "GameMechanics/Units/BattleEffects/BattleEffectComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Unit.h"
UBattleEffectComponent::UBattleEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
bool UBattleEffectComponent::AddEffect(UBattleEffect* Effect)
{
	if (!Effect)
	{
		UE_LOG(LogTemp, Warning, TEXT("BattleEffectComponent::AddEffect - Null effect passed"));
		return false;
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
				if (ExistingEffect->HandleReapply(Effect))
				{
					UE_LOG(LogTemp, Log, TEXT("BattleEffectComponent::AddEffect - Effect reapplied via stacking"));
					return false;
				}
			}
		}
	}
	ActiveEffects.Add(Effect);
	Effect->SetOwner(GetOwnerUnit());
	UE_LOG(LogTemp, Log, TEXT("BattleEffectComponent::AddEffect - Effect added to ActiveEffects, calling OnApplied"));
	Effect->OnApplied();
	UE_LOG(LogTemp, Log, TEXT("BattleEffectComponent::AddEffect - OnApplied called, total effects: %d"), ActiveEffects.Num());
	return true;
}
void UBattleEffectComponent::RemoveEffect(UBattleEffect* Effect)
{
	if (Effect)
	{
		Effect->OnRemoved();
		ActiveEffects.Remove(Effect);
	}
}
void UBattleEffectComponent::ClearAllEffects()
{
	for (UBattleEffect* Effect : ActiveEffects)
	{
		if (Effect)
		{
			Effect->OnRemoved();
		}
	}
	ActiveEffects.Empty();
}
void UBattleEffectComponent::BroadcastTurnStart()
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnTurnStart();
			if (Effect->IsExpired())
			{
				RemoveEffect(Effect);
			}
		}
	}
}
void UBattleEffectComponent::BroadcastTurnEnd()
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnTurnEnd();
			if (Effect->IsExpired())
			{
				RemoveEffect(Effect);
			}
		}
	}
}
void UBattleEffectComponent::BroadcastAttacked(AUnit* Attacker)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnUnitAttacked(Attacker);
			if (Effect->IsExpired())
			{
				RemoveEffect(Effect);
			}
		}
	}
}
void UBattleEffectComponent::BroadcastAttacks(AUnit* Target)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnUnitAttacks(Target);
			if (Effect->IsExpired())
			{
				RemoveEffect(Effect);
			}
		}
	}
}
void UBattleEffectComponent::BroadcastMoved()
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnUnitMoved();
			if (Effect->IsExpired())
			{
				RemoveEffect(Effect);
			}
		}
	}
}
void UBattleEffectComponent::BroadcastDied()
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnUnitDied();
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

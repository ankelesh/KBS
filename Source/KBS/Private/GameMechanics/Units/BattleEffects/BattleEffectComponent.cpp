#include "GameMechanics/Units/BattleEffects/BattleEffectComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Unit.h"
UBattleEffectComponent::UBattleEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBattleEffectComponent::BeginPlay()
{
	Super::BeginPlay();
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	checkf(OwnerUnit, TEXT("BattleEffectComponent must be owned by AUnit"));
	OwnerUnit->OnUnitTurnStart.AddDynamic(this, &UBattleEffectComponent::OnOwnerTurnStart);
	OwnerUnit->OnUnitTurnEnd.AddDynamic(this, &UBattleEffectComponent::OnOwnerTurnEnd);
	OwnerUnit->OnUnitAttacked.AddDynamic(this, &UBattleEffectComponent::OnOwnerAttacked);
	OwnerUnit->OnUnitAttacks.AddDynamic(this, &UBattleEffectComponent::OnOwnerAttacks);
	OwnerUnit->OnUnitMoved.AddDynamic(this, &UBattleEffectComponent::OnOwnerMoved);
	OwnerUnit->OnUnitDied.AddDynamic(this, &UBattleEffectComponent::OnOwnerDied);
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
void UBattleEffectComponent::OnOwnerTurnStart(AUnit* Unit)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnTurnStart();
			if (Effect->IsExpired()) RemoveEffect(Effect);
		}
	}
}

void UBattleEffectComponent::OnOwnerTurnEnd(AUnit* Unit)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnTurnEnd();
			if (Effect->IsExpired()) RemoveEffect(Effect);
		}
	}
}

void UBattleEffectComponent::OnOwnerAttacked(AUnit* Victim, AUnit* Attacker)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnUnitAttacked(Attacker);
			if (Effect->IsExpired()) RemoveEffect(Effect);
		}
	}
}

void UBattleEffectComponent::OnOwnerAttacks(AUnit* Attacker, AUnit* Target)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnUnitAttacks(Target);
			if (Effect->IsExpired()) RemoveEffect(Effect);
		}
	}
}

void UBattleEffectComponent::OnOwnerMoved(AUnit* Unit, const FTacMovementVisualData& MovementData)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnUnitMoved();
			if (Effect->IsExpired()) RemoveEffect(Effect);
		}
	}
}

void UBattleEffectComponent::OnOwnerDied(AUnit* Unit)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Effect->OnUnitDied();
		}
	}
	ClearAllEffects();
}
AUnit* UBattleEffectComponent::GetOwnerUnit() const
{
	return Cast<AUnit>(GetOwner());
}

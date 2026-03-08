#include "GameMechanics/Units/BattleEffects/BattleEffectComponent.h"

#include "Algo/Count.h"
#include "Algo/RemoveIf.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Unit.h"

namespace
{
	bool CheckAndLogEffect(UBattleEffect* Effect, AUnit* Owner)
	{
		if (!Effect)
		{
			UE_LOG(LogTemp, Warning, TEXT("BattleEffectComponent::AddEffect - Null effect passed"));
			return false;
		}
		UE_LOG(LogTemp, Log, TEXT("BattleEffectComponent::AddEffect - Adding effect %s to %s"),
		       *Effect->GetClass()->GetName(),
		       Owner ? *Owner->GetLogName() : TEXT("NULL_OWNER"));
		return true;
	}
}


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

bool UBattleEffectComponent::AddEffect(UBattleEffect* NewEffect)
{
	if (!CheckAndLogEffect(NewEffect, GetOwnerUnit())) { return false; }

	const FName StackId = NewEffect->GetStackingId();

	// No stacking identity — always add
	if (StackId.IsNone())
	{
		ApplyEffect(NewEffect);
		return true;
	}

	UBattleEffect* Existing = FindByStackingId(StackId);
	if (!Existing)
	{
		ApplyEffect(NewEffect);
		return true;
	}

	const UBattleEffectDataAsset* Config = NewEffect->GetConfig();
	switch (Config->StackPolicy)
	{
	case EEffectStackPolicy::Unique:
		return false;

	case EEffectStackPolicy::AlwaysReplaced:
		RemoveEffect(Existing);
		ApplyEffect(NewEffect);
		return true;

	case EEffectStackPolicy::RefreshOld:
		Existing->RefreshDuration(NewEffect->GetDuration());
		return false;

	case EEffectStackPolicy::RefreshOrReplace:
		{
			const EReapplyDecision Decision = Existing->HandleReapply(NewEffect);
			ExecuteReapplyDecision(Decision, Existing, NewEffect);
			return Decision == EReapplyDecision::New;
		}

	case EEffectStackPolicy::StackInfinite:
		ApplyEffect(NewEffect);
		return true;

	case EEffectStackPolicy::Stack:
		if (CountByStackingId(StackId) >= Config->MaxStacks)
			return false;
		ApplyEffect(NewEffect);
		return true;

	case EEffectStackPolicy::Custom:
		{
			const EReapplyDecision Decision = Existing->HandleReapply(NewEffect);
			ExecuteReapplyDecision(Decision, Existing, NewEffect);
			return Decision == EReapplyDecision::New;
		}
	}
	return false;
}

void UBattleEffectComponent::RemoveEffect(UBattleEffect* Effect)
{
	if (Effect)
	{
		Effect->OnRemoved();
		ActiveEffects.Remove(Effect);
	}
}

void UBattleEffectComponent::RemoveEffect(FName StackingId)
{
	auto ToRemove = FindByStackingId(StackingId);
	if (ToRemove)
		RemoveEffect(ToRemove);
}

void UBattleEffectComponent::RemoveEffect(const FGuid EffectId)
{
	auto Effect = ActiveEffects.FindByPredicate([=](const UBattleEffect* Effect){return Effect->GetEffectId() == EffectId; });
	if (Effect)
	{
		(*Effect)->OnRemoved();
		ActiveEffects.RemoveSingle(*Effect);
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

bool UBattleEffectComponent::HasEffectWithTag(const FGameplayTag& Tag) const
{
	for (auto& Effect : ActiveEffects)
	{
		if (Effect->HasTag(Tag))
			return true;
	}
	return false;
}

int32 UBattleEffectComponent::CountEffectsWithTag(const FGameplayTag& Tag) const
{
	return Algo::CountIf(ActiveEffects, [=](const UBattleEffect* Effect) { return Effect->HasTag(Tag); });
}

TArray<UBattleEffect*> UBattleEffectComponent::GetEffectsMatching(TOptional<FGameplayTag> Tag,
                                                                  TOptional<EEffectPolarity> PolarityFilter,
                                                                  TOptional<bool> DispellableFilter) const
{
	TArray<UBattleEffect*> FilteredEffects;
	Algo::CopyIf(ActiveEffects, FilteredEffects, [=](const UBattleEffect* Effect)
	{
		if (Tag.IsSet() && !Effect->HasTag(Tag.GetValue()))
			return false;

		if (PolarityFilter.IsSet() && Effect->GetPolarity() != PolarityFilter.GetValue())
			return false;

		if (DispellableFilter.IsSet() && Effect->IsDispellable() != DispellableFilter.GetValue())
			return false;

		return true;
	});
	return FilteredEffects;
}

int32 UBattleEffectComponent::DispelEffects(TOptional<EEffectPolarity> PolarityFilter,
                                            TOptional<FGameplayTag> TagFilter, int32 MaxToRemove)
{
	TArray<UBattleEffect*> FilteredEffects{GetEffectsMatching(TagFilter, PolarityFilter, true)};
	TArray<FGuid> GuidsToRemove;
	Algo::Transform(FilteredEffects, GuidsToRemove, [=](UBattleEffect* Effect) {return Effect->GetEffectId();});
	int32 EffectsToRemove{FMath::Min(MaxToRemove, GuidsToRemove.Num())};
	for (int i = 0; i < EffectsToRemove; i++)
	{
		RemoveEffect(GuidsToRemove[i]);
	}
	return EffectsToRemove;
}

UBattleEffect* UBattleEffectComponent::FindByStackingId(FName StackingId) const
{
	auto Found = Algo::FindByPredicate(ActiveEffects, [=](const UBattleEffect* Effect)
	{
		return Effect->GetStackingId() == StackingId;
	});
	return Found? *Found : nullptr;
}

int32 UBattleEffectComponent::CountByStackingId(FName StackingId) const
{
	return Algo::CountIf(ActiveEffects, [=](const UBattleEffect* Effect)
	{
		return Effect->GetStackingId() == StackingId;
	});
}

void UBattleEffectComponent::ApplyEffect(UBattleEffect* Effect)
{
	ActiveEffects.Add(Effect);
	Effect->SetOwner(GetOwnerUnit());
	Effect->OnApplied();
}

void UBattleEffectComponent::ExecuteReapplyDecision(EReapplyDecision Decision, UBattleEffect* OldEffect,
                                                    UBattleEffect* NewEffect)
{
	switch (Decision)
	{
	case EReapplyDecision::Old:
		break;
	case EReapplyDecision::New:
		RemoveEffect(OldEffect);
		ApplyEffect(NewEffect);
		break;
	case EReapplyDecision::OverrideDuration:
		OldEffect->RefreshDuration(NewEffect->GetDuration());
		break;
	case EReapplyDecision::DoNothing:
		break;
	}
}

void UBattleEffectComponent::BroadcastToEffects(TFunctionRef<void(UBattleEffect*)> Notify, bool bCheckExpiry)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (UBattleEffect* Effect = ActiveEffects[i])
		{
			Notify(Effect);
			if (bCheckExpiry && Effect->IsExpired()) RemoveEffect(Effect);
		}
	}
}

void UBattleEffectComponent::OnOwnerTurnStart(AUnit* Unit)
{
	BroadcastToEffects([](UBattleEffect* E) { E->OnTurnStart(); });
}

void UBattleEffectComponent::OnOwnerTurnEnd(AUnit* Unit)
{
	BroadcastToEffects([](UBattleEffect* E) { E->OnTurnEnd(); });
}

void UBattleEffectComponent::OnOwnerAttacked(AUnit* Victim, AUnit* Attacker)
{
	BroadcastToEffects([Attacker](UBattleEffect* E) { E->OnUnitAttacked(Attacker); });
}

void UBattleEffectComponent::OnOwnerAttacks(AUnit* Attacker, AUnit* Target)
{
	BroadcastToEffects([Target](UBattleEffect* E) { E->OnUnitAttacks(Target); });
}

void UBattleEffectComponent::OnOwnerMoved(AUnit* Unit, const FTacMovementVisualData& MovementData)
{
	BroadcastToEffects([](UBattleEffect* E) { E->OnUnitMoved(); });
}

void UBattleEffectComponent::OnOwnerDied(AUnit* Unit)
{
	BroadcastToEffects([](UBattleEffect* E) { E->OnUnitDied(); }, false);
	ClearAllEffects();
}

AUnit* UBattleEffectComponent::GetOwnerUnit() const
{
	return Cast<AUnit>(GetOwner());
}

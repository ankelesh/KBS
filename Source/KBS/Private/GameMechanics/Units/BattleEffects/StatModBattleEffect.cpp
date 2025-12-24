#include "GameMechanics/Units/BattleEffects/StatModBattleEffect.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/StatModBattleEffectDataAsset.h"

void UStatModBattleEffect::Initialize(UBattleEffectDataAsset* InConfig, UWeapon* InWeapon, AUnit* InAttacker)
{
	Super::Initialize(InConfig, InWeapon, InAttacker);
	UStatModBattleEffectDataAsset* StatModConfig = Cast<UStatModBattleEffectDataAsset>(InConfig);
	if (StatModConfig)
	{
		RemainingTurns = StatModConfig->Duration;
	}
}

void UStatModBattleEffect::OnTurnEnd(AUnit* Owner)
{
	DecrementDuration();
	UE_LOG(LogTemp, Log, TEXT("%s: StatMod effect '%s' tick (%d turns remaining)"),
		*Owner->GetName(),
		*Config->Name.ToString(),
		RemainingTurns);
}

void UStatModBattleEffect::OnApplied(AUnit* Owner)
{
	if (!Owner)
	{
		return;
	}
	ApplyStatModifications(Owner);
	UE_LOG(LogTemp, Log, TEXT("%s: StatMod effect '%s' applied for %d turns"),
		*Owner->GetName(),
		*Config->Name.ToString(),
		RemainingTurns);
}

void UStatModBattleEffect::OnRemoved(AUnit* Owner)
{
	if (!Owner)
	{
		return;
	}
	RemoveStatModifications(Owner);
	UE_LOG(LogTemp, Log, TEXT("%s: StatMod effect '%s' removed"),
		*Owner->GetName(),
		*Config->Name.ToString());
}

bool UStatModBattleEffect::HandleReapply(UBattleEffect* NewEffect, AUnit* Owner)
{
	UStatModBattleEffectDataAsset* StatModConfig = GetStatModConfig();
	UStatModBattleEffectDataAsset* NewStatModConfig = NewEffect ? Cast<UStatModBattleEffectDataAsset>(NewEffect->GetConfig()) : nullptr;
	if (!NewEffect || !StatModConfig || !NewStatModConfig)
	{
		return false;
	}
	RemoveStatModifications(Owner);
	Config = NewStatModConfig;
	RemainingTurns = NewStatModConfig->Duration;
	ApplyStatModifications(Owner);
	UE_LOG(LogTemp, Log, TEXT("%s: StatMod effect '%s' reapplied, duration reset to %d"),
		*Owner->GetName(),
		*Config->Name.ToString(),
		RemainingTurns);
	return true;
}

void UStatModBattleEffect::ApplyStatModifications(AUnit* Owner)
{
	UStatModBattleEffectDataAsset* StatModConfig = GetStatModConfig();
	if (!Owner || !StatModConfig)
	{
		return;
	}
	FUnitCoreStats& ModifiedStats = const_cast<FUnitCoreStats&>(Owner->GetModifiedStats());
	ModifiedStats.MaxHealth += StatModConfig->StatModifications.MaxHealth;
	ModifiedStats.Initiative += StatModConfig->StatModifications.Initiative;
	ModifiedStats.Accuracy += StatModConfig->StatModifications.Accuracy;
	for (const EDamageSource& Immunity : StatModConfig->StatModifications.Defense.Immunities)
	{
		ModifiedStats.Defense.Immunities.Add(Immunity);
	}
	for (const auto& ArmorPair : StatModConfig->StatModifications.Defense.Armour)
	{
		float* ExistingArmor = ModifiedStats.Defense.Armour.Find(ArmorPair.Key);
		if (ExistingArmor)
		{
			*ExistingArmor = FMath::Clamp(*ExistingArmor + ArmorPair.Value, 0.0f, 0.9f);
		}
		else
		{
			ModifiedStats.Defense.Armour.Add(ArmorPair.Key, FMath::Clamp(ArmorPair.Value, 0.0f, 0.9f));
		}
	}
	ModifiedStats.Defense.DamageReduction += StatModConfig->StatModifications.Defense.DamageReduction;
}

void UStatModBattleEffect::RemoveStatModifications(AUnit* Owner)
{
	UStatModBattleEffectDataAsset* StatModConfig = GetStatModConfig();
	if (!Owner || !StatModConfig)
	{
		return;
	}
	const FUnitCoreStats& BaseStats = Owner->GetBaseStats();
	FUnitCoreStats& ModifiedStats = const_cast<FUnitCoreStats&>(Owner->GetModifiedStats());
	ModifiedStats.MaxHealth -= StatModConfig->StatModifications.MaxHealth;
	ModifiedStats.Initiative -= StatModConfig->StatModifications.Initiative;
	ModifiedStats.Accuracy -= StatModConfig->StatModifications.Accuracy;
	for (const EDamageSource& Immunity : StatModConfig->StatModifications.Defense.Immunities)
	{
		if (!BaseStats.Defense.Immunities.Contains(Immunity))
		{
			ModifiedStats.Defense.Immunities.Remove(Immunity);
		}
	}
	for (const auto& ArmorPair : StatModConfig->StatModifications.Defense.Armour)
	{
		float* ExistingArmor = ModifiedStats.Defense.Armour.Find(ArmorPair.Key);
		if (ExistingArmor)
		{
			*ExistingArmor = FMath::Clamp(*ExistingArmor - ArmorPair.Value, 0.0f, 0.9f);
		}
	}
	ModifiedStats.Defense.DamageReduction -= StatModConfig->StatModifications.Defense.DamageReduction;
}

#include "GameMechanics/Units/BattleEffects/StatModBattleEffect.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/StatModBattleEffectDataAsset.h"

void UStatModBattleEffect::Initialize(UBattleEffectDataAsset* InConfig)
{
	Super::Initialize(InConfig);
	UStatModBattleEffectDataAsset* StatModConfig = Cast<UStatModBattleEffectDataAsset>(InConfig);
	if (StatModConfig)
	{
		Duration = StatModConfig->Duration;
	}
}

void UStatModBattleEffect::OnTurnEnd()
{
	DecrementDuration();
	UE_LOG(LogTemp, Log, TEXT("%s: StatMod effect '%s' tick (%d turns remaining)"),
		*Owner->GetName(),
		*Config->Name.ToString(),
		Duration);
}

void UStatModBattleEffect::OnApplied()
{
	if (!Owner)
	{
		return;
	}
	ApplyStatModifications();
	UE_LOG(LogTemp, Log, TEXT("%s: StatMod effect '%s' applied for %d turns"),
		*Owner->GetName(),
		*Config->Name.ToString(),
		Duration);
}

void UStatModBattleEffect::OnRemoved()
{
	if (!Owner)
	{
		return;
	}
	RemoveStatModifications();
	UE_LOG(LogTemp, Log, TEXT("%s: StatMod effect '%s' removed"),
		*Owner->GetName(),
		*Config->Name.ToString());
}

bool UStatModBattleEffect::HandleReapply(UBattleEffect* NewEffect)
{
	UStatModBattleEffectDataAsset* StatModConfig = GetStatModConfig();
	UStatModBattleEffectDataAsset* NewStatModConfig = NewEffect ? Cast<UStatModBattleEffectDataAsset>(NewEffect->GetConfig()) : nullptr;
	if (!NewEffect || !StatModConfig || !NewStatModConfig)
	{
		return false;
	}
	RemoveStatModifications();
	Config = NewStatModConfig;
	Duration = NewStatModConfig->Duration;
	ApplyStatModifications();
	UE_LOG(LogTemp, Log, TEXT("%s: StatMod effect '%s' reapplied, duration reset to %d"),
		*Owner->GetName(),
		*Config->Name.ToString(),
		Duration);
	return true;
}

void UStatModBattleEffect::ApplyStatModifications()
{
	UStatModBattleEffectDataAsset* Config = GetStatModConfig();
	if (!Owner || !Config)
	{
		return;
	}

	FUnitCoreStats& Stats = Owner->GetStats();
	const FGuid& EffId = GetEffectId();

	// Apply stat modifiers (0 = no modification)
	AppliedMaxHealthMod = Config->MaxHealthModifier;
	if (AppliedMaxHealthMod != 0)
	{
		Stats.Health.AddMaxModifier(EffId, AppliedMaxHealthMod, true);
	}

	AppliedInitiativeMod = Config->InitiativeModifier;
	if (AppliedInitiativeMod != 0)
	{
		Stats.Initiative.AddFlatModifier(EffId, AppliedInitiativeMod);
	}

	AppliedAccuracyMod = Config->AccuracyModifier;
	if (AppliedAccuracyMod != 0)
	{
		Stats.Accuracy.AddFlatModifier(EffId, AppliedAccuracyMod);
	}

	// Apply immunities
	AppliedImmunities = Config->ImmunitiesToGrant.Array();
	for (EDamageSource Immunity : AppliedImmunities)
	{
		Stats.Defense.Immunities.AddModifier(EffId, Immunity, true);
	}

	// Apply armour modifiers
	for (const auto& ArmorPair : Config->ArmourModifiers)
	{
		if (ArmorPair.Value != 0)
		{
			AppliedArmourMods.Add(ArmorPair.Key, ArmorPair.Value);
			Stats.Defense.Armour.AddFlatModifier(EffId, ArmorPair.Value, ArmorPair.Key);
		}
	}
}

void UStatModBattleEffect::RemoveStatModifications()
{
	if (!Owner)
	{
		return;
	}

	FUnitCoreStats& Stats = Owner->GetStats();
	const FGuid& EffId = GetEffectId();

	// Remove using cached amounts
	if (AppliedMaxHealthMod != 0)
	{
		Stats.Health.RemoveMaxModifier(EffId, AppliedMaxHealthMod);
		AppliedMaxHealthMod = 0;
	}

	if (AppliedInitiativeMod != 0)
	{
		Stats.Initiative.RemoveFlatModifier(EffId, AppliedInitiativeMod);
		AppliedInitiativeMod = 0;
	}

	if (AppliedAccuracyMod != 0)
	{
		Stats.Accuracy.RemoveFlatModifier(EffId, AppliedAccuracyMod);
		AppliedAccuracyMod = 0;
	}

	// Remove immunities
	for (EDamageSource Immunity : AppliedImmunities)
	{
		Stats.Defense.Immunities.RemoveModifier(EffId, Immunity, true);
	}
	AppliedImmunities.Empty();

	// Remove armour modifications
	for (const auto& ArmorPair : AppliedArmourMods)
	{
		Stats.Defense.Armour.RemoveFlatModifier(EffId, ArmorPair.Value, ArmorPair.Key);
	}
	AppliedArmourMods.Empty();
}

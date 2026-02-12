#include "GameMechanics/Units/UnitDisplayData.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Stats/UnitStats.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"

static TArray<EDamageSource> DamageSources {
	EDamageSource::Physical,
	EDamageSource::Fire,
	EDamageSource::Earth, 
	EDamageSource::Air,
	EDamageSource::Water, 
	EDamageSource::Life,
	EDamageSource::Death, 
	EDamageSource::Mind
};


FString DamageSourceToString(EDamageSource Source)
{
	switch (Source)
	{
		case EDamageSource::None: return TEXT("None");
		case EDamageSource::Physical: return TEXT("Physical");
		case EDamageSource::Fire: return TEXT("Fire");
		case EDamageSource::Earth: return TEXT("Earth");
		case EDamageSource::Air: return TEXT("Air");
		case EDamageSource::Water: return TEXT("Water");
		case EDamageSource::Life: return TEXT("Life");
		case EDamageSource::Death: return TEXT("Death");
		case EDamageSource::Mind: return TEXT("Mind");
		default: return TEXT("Unknown");
	}
}
FString TargetReachToString(ETargetReach Reach)
{
	switch (Reach)
	{
		case ETargetReach::None: return TEXT("None");
		case ETargetReach::Self: return TEXT("Self");
		case ETargetReach::ClosestEnemies: return TEXT("Closest Enemies");
		case ETargetReach::AnyEnemy: return TEXT("Any Enemy");
		case ETargetReach::AllEnemies: return TEXT("All Enemies");
		case ETargetReach::Area: return TEXT("Area");
		case ETargetReach::AnyFriendly: return TEXT("Any Friendly");
		case ETargetReach::AllFriendlies: return TEXT("All Friendlies");
		case ETargetReach::EmptyCell: return TEXT("Empty Cell");
		case ETargetReach::EmptyCellOrFriendly: return TEXT("Empty Cell Or Friendly");
		case ETargetReach::AnyCorpse: return TEXT("Any Corpse");
		case ETargetReach::FriendlyCorpse: return TEXT("Friendly Corpse");
		case ETargetReach::EnemyCorpse: return TEXT("Enemy Corpse");
		case ETargetReach::AnyNonBlockedCorpse: return TEXT("Any Non-Blocked Corpse");
		case ETargetReach::FriendlyNonBlockedCorpse: return TEXT("Friendly Non-Blocked Corpse");
		case ETargetReach::EnemyNonBlockedCorpse: return TEXT("Enemy Non-Blocked Corpse");
		default: return TEXT("Unknown");
	}
}
FWeaponDisplayData ConvertWeapon(const UWeapon* Weapon)
{
	FWeaponDisplayData DisplayData;
	if (!Weapon)
	{
		return DisplayData;
	}
	const UWeaponDataAsset* Config = Weapon->GetConfig();
	if (Config)
	{
		DisplayData.WeaponName = Config->Name.ToString();
	}
	const FWeaponStats& Stats = Weapon->GetStats();
	DisplayData.TargetType = TargetReachToString(Stats.TargetReach);
	DisplayData.Damage = Stats.BaseDamage.GetValue();
	TArray<FString> DamageTypeArray = ConvertDamageSourceSet(Stats.DamageSources);
	DisplayData.DamageTypes = FString::Join(DamageTypeArray, TEXT(" + "));
	return DisplayData;
}
TArray<FString> ConvertActiveEffects(const TArray<TObjectPtr<UBattleEffect>>& Effects)
{
	TArray<FString> Result;
	for (const TObjectPtr<UBattleEffect>& Effect : Effects)
	{
		if (Effect)
		{
			Result.Add(Effect->GetEffectName().ToString());
		}
	}
	return Result;
}
FString ConvertArmourMap(const TMap<EDamageSource, int32>& ArmourMap)
{
	TArray<FString> ArmourParts;
	for (const auto& Pair : ArmourMap)
	{
		if (Pair.Value > 0)
		{
			FString SourceName = DamageSourceToString(Pair.Key);
			ArmourParts.Add(FString::Printf(TEXT("%s: %d%%"), *SourceName, Pair.Value));
		}
	}
	return FString::Join(ArmourParts, TEXT(", "));
}
FUnitDisplayData BuildUnitDisplayData(
	const FString& UnitName,
	float CurrentHealth,
	const FUnitCoreStats& Stats,
	UTexture2D* PortraitTexture,
	const TArray<TObjectPtr<UBattleEffect>>& ActiveEffects,
	const TArray<TObjectPtr<UWeapon>>& Weapons,
	ETeamSide TeamSide
)
{
	FUnitDisplayData DisplayData;
	DisplayData.UnitName = UnitName;
	DisplayData.CurrentHealth = CurrentHealth;
	DisplayData.MaxHealth = Stats.Health.GetMaximum();
	DisplayData.Initiative = Stats.Initiative.GetValue();
	DisplayData.Accuracy = Stats.Accuracy.GetValue() / 100.0f;
	DisplayData.Level = 0;
	DisplayData.Experience = 0;
	DisplayData.ExperienceToNextLevel = 0;
	DisplayData.PortraitTexture = PortraitTexture;
	DisplayData.BelongsToAttackerTeam = (TeamSide == ETeamSide::Attacker);
	TArray<FString> EffectArray = ConvertActiveEffects(ActiveEffects);
	DisplayData.ActiveEffectNames = FString::Join(EffectArray, TEXT(", "));
	for (const TObjectPtr<UWeapon>& Weapon : Weapons)
	{
		if (Weapon)
		{
			DisplayData.Weapons.Add(ConvertWeapon(Weapon));
		}
	}
	TArray<FString> ImmunitiesArray = ConvertImmunityMap(Stats.Defense.Immunities);
	DisplayData.Immunities = FString::Join(ImmunitiesArray, TEXT(", "));
	TArray<FString> WardsArray = ConvertWardMap(Stats.Defense.Wards);
	DisplayData.Wards = FString::Join(WardsArray, TEXT(", "));
	DisplayData.Armour = ConvertArmourMap(Stats.Defense.Armour);
	DisplayData.DamageReduction = Stats.Defense.DamageReduction.GetValue();
	DisplayData.bIsDefending = Stats.Status.IsDefending();
	return DisplayData;
}

FString ConvertArmourMap(const FUnitArmour& ArmourMap)
{
	TArray<FString> ArmourParts;
	ArmourParts.Reserve(8);
	for (auto DamageSrc : DamageSources)
	{
		FString SourceName = DamageSourceToString(DamageSrc);
		ArmourParts.Add(FString::Printf(TEXT("%s: %d%%"), *SourceName, ArmourMap.GetValue(DamageSrc)));
	}
	return FString::Join(ArmourParts, TEXT(", "));
}
TArray<FString> ConvertImmunityMap(const FUnitImmunities& ImmunityMap)
{
	TArray<FString> ImmunitiesParts;
	ImmunitiesParts.Reserve(8);
	for (auto DamageSrc : DamageSources)
	{
		if (ImmunityMap.IsImmuneTo(DamageSrc))
		{
			FString SourceName = DamageSourceToString(DamageSrc);
			ImmunitiesParts.Add(SourceName);
		}
	}
	return ImmunitiesParts;
}
TArray<FString> ConvertWardMap(const FUnitWards& WardMap)
{
	TArray<FString> WardParts;
	WardParts.Reserve(8);
	for (auto DamageSrc : DamageSources)
	{
		if (WardMap.HasWardFor(DamageSrc))
		{
			FString SourceName = DamageSourceToString(DamageSrc);
			WardParts.Add(SourceName);
		}
	}
	return WardParts;
}
TArray<FString> ConvertDamageSourceSet(const FDamageSourceSetStat& DamageSourceSet)
{
	TArray<FString> DamageSourceSetParts;
	DamageSourceSetParts.Reserve(8);
	for (auto DamageSrc : DamageSourceSet.GetValue())
	{
		FString DamageSourceName = DamageSourceToString(DamageSrc);
		DamageSourceSetParts.Add(DamageSourceName);
	}
	return DamageSourceSetParts;
}
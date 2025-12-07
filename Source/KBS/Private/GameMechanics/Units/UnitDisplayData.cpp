#include "GameMechanics/Units/UnitDisplayData.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"

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
		case ETargetReach::ClosestEnemies: return TEXT("Closest Enemies");
		case ETargetReach::AnyEnemy: return TEXT("Any Enemy");
		case ETargetReach::AllEnemies: return TEXT("All Enemies");
		case ETargetReach::Area: return TEXT("Area");
		case ETargetReach::AnyFriendly: return TEXT("Any Friendly");
		case ETargetReach::AllFriendlies: return TEXT("All Friendlies");
		case ETargetReach::EmptyCell: return TEXT("Empty Cell");
		case ETargetReach::EmptyCellOrFriendly: return TEXT("Empty Cell Or Friendly");
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
	DisplayData.Damage = Stats.BaseDamage;

	TArray<FString> DamageTypeArray = ConvertDamageSourceSet(Stats.DamageSources);
	DisplayData.DamageTypes = FString::Join(DamageTypeArray, TEXT(" + "));

	return DisplayData;
}

TArray<FString> ConvertDamageSourceSet(const TSet<EDamageSource>& SourceSet)
{
	TArray<FString> Result;
	for (EDamageSource Source : SourceSet)
	{
		Result.Add(DamageSourceToString(Source));
	}
	return Result;
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

FString ConvertArmourMap(const TMap<EDamageSource, float>& ArmourMap)
{
	TArray<FString> ArmourParts;
	for (const auto& Pair : ArmourMap)
	{
		if (Pair.Value > 0.0f)
		{
			FString SourceName = DamageSourceToString(Pair.Key);
			int32 Percentage = FMath::RoundToInt(Pair.Value * 100.0f);
			ArmourParts.Add(FString::Printf(TEXT("%s: %d%%"), *SourceName, Percentage));
		}
	}
	return FString::Join(ArmourParts, TEXT(", "));
}

FUnitDisplayData BuildUnitDisplayData(
	const FString& UnitName,
	float CurrentHealth,
	const FUnitCoreStats& ModifiedStats,
	const FUnitProgressionData& Progression,
	UTexture2D* PortraitTexture,
	const TArray<TObjectPtr<UBattleEffect>>& ActiveEffects,
	const TArray<TObjectPtr<UWeapon>>& Weapons,
	ETeamSide TeamSide
)
{
	FUnitDisplayData DisplayData;

	DisplayData.UnitName = UnitName;
	DisplayData.CurrentHealth = CurrentHealth;
	DisplayData.MaxHealth = ModifiedStats.MaxHealth;
	DisplayData.Initiative = ModifiedStats.Initiative;
	DisplayData.Accuracy = ModifiedStats.Accuracy;
	DisplayData.Level = Progression.LevelOnCurrentTier;
	DisplayData.Experience = Progression.TotalExperience;
	DisplayData.ExperienceToNextLevel = Progression.ExperienceToNextLevel;
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

	// Defense stats
	TArray<FString> ImmunitiesArray = ConvertDamageSourceSet(ModifiedStats.Defense.Immunities);
	DisplayData.Immunities = FString::Join(ImmunitiesArray, TEXT(", "));

	TArray<FString> WardsArray = ConvertDamageSourceSet(ModifiedStats.Defense.Wards);
	DisplayData.Wards = FString::Join(WardsArray, TEXT(", "));

	DisplayData.Armour = ConvertArmourMap(ModifiedStats.Defense.Armour);
	DisplayData.DamageReduction = ModifiedStats.Defense.DamageReduction;

	return DisplayData;
}

#include "GameMechanics/Tactical/DamageCalculator.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/UnitStats.h"
void UDamageCalculator::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetStatistics();
}
void UDamageCalculator::Deinitialize()
{
	Super::Deinitialize();
}
float UDamageCalculator::CalculateHitChance(AUnit* Attacker, UWeapon* Weapon, AUnit* Target)
{
	if (!Attacker || !Weapon || !Target)
	{
		return 0.0f;
	}
	const FUnitCoreStats& AttackerStats = Attacker->GetStats();
	const FWeaponStats& WeaponStats = Weapon->GetStats();
	float HitChance = AttackerStats.Accuracy * WeaponStats.AccuracyMultiplier * 100.0f;
	return FMath::Clamp(HitChance, 0.0f, 100.0f);
}
FDamageResult UDamageCalculator::CalculateDamage(AUnit* Attacker, UWeapon* Weapon, AUnit* Target)
{
	FDamageResult Result;
	if (!Attacker || !Weapon || !Target)
	{
		return Result;
	}
	const FWeaponStats& WeaponStats = Weapon->GetStats();
	const FUnitCoreStats& TargetStats = Target->GetStats();
	const FUnitDefenseStats& Defense = TargetStats.Defense;
	EDamageSource BestSource = SelectBestDamageSource(WeaponStats.DamageSources, Target);
	Result.DamageSource = BestSource;
	if (BestSource == EDamageSource::None)
	{
		return Result;
	}
	int32 BaseDamage = WeaponStats.BaseDamage;
	if (Defense.Immunities.Contains(BestSource))
	{
		Result.Damage = 0;
		Result.DamageBlocked = BaseDamage;
		return Result;
	}
	if (Defense.Wards.Contains(BestSource))
	{
		Result.Damage = 0;
		Result.DamageBlocked = BaseDamage;
		return Result;
	}
	float ArmorValue = 0.0f;
	if (Defense.Armour.Contains(BestSource))
	{
		ArmorValue = FMath::Clamp(Defense.Armour[BestSource], 0.0f, 0.9f);
	}
	float DamageAfterArmor = BaseDamage * (1.0f - ArmorValue);
	float FinalDamage = DamageAfterArmor - Defense.DamageReduction;
	if (Defense.bIsDefending)
	{
		FinalDamage *= 0.5f;
	}
	Result.Damage = FMath::RoundToInt(FinalDamage);
	Result.DamageBlocked = BaseDamage - Result.Damage;
	return Result;
}
float UDamageCalculator::CalculateEffectApplication(AUnit* Attacker, UBattleEffect* Effect, AUnit* Target)
{
	if (!Attacker || !Effect || !Target)
	{
		return 0.0f;
	}
	const FUnitCoreStats& AttackerStats = Attacker->GetStats();
	const FUnitDefenseStats& Defense = Target->GetStats().Defense;
	EDamageSource EffectSource = Effect->GetDamageSource();
	if (Defense.Immunities.Contains(EffectSource))
	{
		return 0.0f;
	}
	if (Defense.Wards.Contains(EffectSource))
	{
		return 0.0f;
	}
	float ApplicationChance = AttackerStats.Accuracy * 100.0f;
	return FMath::Clamp(ApplicationChance, 0.0f, 100.0f);
}
UWeapon* UDamageCalculator::SelectWeapon(AUnit* Attacker, AUnit* Target, ETargetReach ExpectedReach)
{
	if (!Attacker || !Target)
	{
		return nullptr;
	}
	const TArray<TObjectPtr<UWeapon>>& Weapons = Attacker->GetWeapons();
	if (Weapons.Num() == 0)
	{
		return nullptr;
	}
	if (Weapons.Num() == 1)
	{
		return Weapons[0];
	}
	TArray<UWeapon*> ValidWeapons;
	for (UWeapon* Weapon : Weapons)
	{
		if (!Weapon)
		{
			continue;
		}
		const FWeaponStats& Stats = Weapon->GetStats();
		if (Stats.TargetReach != ExpectedReach && Stats.TargetReach != ETargetReach::AnyEnemy)
		{
			continue;
		}
		ValidWeapons.Add(Weapon);
	}
	if (ValidWeapons.Num() == 0)
	{
		return nullptr;
	}
	UWeapon* BestWeapon = nullptr;
	int32 BestDamage = -1;
	for (UWeapon* Weapon : ValidWeapons)
	{
		FDamageResult DamageResult = CalculateDamage(Attacker, Weapon, Target);
		if (DamageResult.Damage > BestDamage)
		{
			BestDamage = DamageResult.Damage;
			BestWeapon = Weapon;
		}
	}
	return BestWeapon;
}
UWeapon* UDamageCalculator::SelectMaxReachWeapon(AUnit* Unit)
{
	if (!Unit)
	{
		return nullptr;
	}
	const TArray<TObjectPtr<UWeapon>>& Weapons = Unit->GetWeapons();
	if (Weapons.Num() == 0)
	{
		return nullptr;
	}
	if (Weapons.Num() == 1)
	{
		return Weapons[0];
	}
	auto GetReachScore = [](ETargetReach Reach) -> int32
	{
		switch (Reach)
		{
			case ETargetReach::AllEnemies:         return 100;
			case ETargetReach::Area:               return 80;
			case ETargetReach::AnyEnemy:           return 60;
			case ETargetReach::AllFriendlies:      return 50;
			case ETargetReach::AnyFriendly:        return 40;
			case ETargetReach::ClosestEnemies:     return 30;
			case ETargetReach::EmptyCellOrFriendly: return 20;
			case ETargetReach::EmptyCell:          return 10;
			case ETargetReach::Self:               return 5;
			case ETargetReach::None:               return 0;
			default:                               return 0;
		}
	};
	UWeapon* BestWeapon = nullptr;
	int32 BestScore = -1;
	for (UWeapon* Weapon : Weapons)
	{
		if (!Weapon)
		{
			continue;
		}
		const FWeaponStats& Stats = Weapon->GetStats();
		int32 Score = GetReachScore(Stats.TargetReach);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestWeapon = Weapon;
		}
	}
	return BestWeapon;
}
FPreviewHitResult UDamageCalculator::PreviewDamage(AUnit* Attacker, AUnit* Target, ETargetReach ExpectedReach)
{
	FPreviewHitResult Preview;
	if (!Attacker || !Target)
	{
		return Preview;
	}
	UWeapon* SelectedWeapon = SelectWeapon(Attacker, Target, ExpectedReach);
	if (!SelectedWeapon)
	{
		return Preview;
	}
	Preview.HitProbability = CalculateHitChance(Attacker, SelectedWeapon, Target);
	Preview.DamageResult = CalculateDamage(Attacker, SelectedWeapon, Target);
	const TArray<TObjectPtr<UBattleEffect>>& Effects = SelectedWeapon->GetEffects();
	if (Effects.Num() > 0)
	{
		float TotalEffectChance = 0.0f;
		for (UBattleEffect* Effect : Effects)
		{
			if (Effect)
			{
				TotalEffectChance += CalculateEffectApplication(Attacker, Effect, Target);
			}
		}
		Preview.EffectApplicationProbability = TotalEffectChance / Effects.Num();
	}
	return Preview;
}
FCombatHitResult UDamageCalculator::ProcessHit(AUnit* Attacker, AUnit* Target, ETargetReach ExpectedReach)
{
	FCombatHitResult HitResult;
	HitResult.TargetUnit = Target;
	if (!Attacker || !Target)
	{
		return HitResult;
	}
	UWeapon* SelectedWeapon = SelectWeapon(Attacker, Target, ExpectedReach);
	if (!SelectedWeapon)
	{
		return HitResult;
	}
	float HitChance = CalculateHitChance(Attacker, SelectedWeapon, Target);
	if (!PerformAccuracyRoll(HitChance))
	{
		HitResult.bHit = false;
		UE_LOG(LogTemp, Warning, TEXT("MISS: %s attacked %s with %.1f%% hit chance - Attack missed!"),
			*Attacker->GetName(), *Target->GetName(), HitChance);
		return HitResult;
	}
	HitResult.bHit = true;
	UE_LOG(LogTemp, Log, TEXT("HIT: %s attacked %s with %.1f%% hit chance - Attack succeeded!"),
		*Attacker->GetName(), *Target->GetName(), HitChance);
	FDamageResult DamageResult = CalculateDamage(Attacker, SelectedWeapon, Target);
	HitResult.Damage = DamageResult.Damage;
	HitResult.DamageSource = DamageResult.DamageSource;
	HitResult.DamageBlocked = DamageResult.DamageBlocked;
	const FUnitDefenseStats& Defense = Target->GetStats().Defense;
	if (Defense.Wards.Contains(DamageResult.DamageSource))
	{
		HitResult.WardSpent = DamageResult.DamageSource;
	}
	Target->TakeHit(DamageResult);
	const TArray<TObjectPtr<UBattleEffect>>& Effects = SelectedWeapon->GetEffects();
	for (UBattleEffect* Effect : Effects)
	{
		if (!Effect)
		{
			continue;
		}
		float EffectChance = CalculateEffectApplication(Attacker, Effect, Target);
		if (PerformAccuracyRoll(EffectChance))
		{
			Target->ApplyEffect(Effect);
			HitResult.EffectsApplied.Add(Effect);
		}
	}
	return HitResult;
}
TArray<FCombatHitResult> UDamageCalculator::ProcessAttack(const FHitInstance& HitInstance)
{
	TArray<FCombatHitResult> Results;
	if (!HitInstance.SourceUnit)
	{
		return Results;
	}
	for (AUnit* Target : HitInstance.TargetUnits)
	{
		if (!Target)
		{
			continue;
		}
		FCombatHitResult HitResult = ProcessHit(HitInstance.SourceUnit, Target, HitInstance.SelectedRange);
		ProcessStatistics(HitResult, true);
		Results.Add(HitResult);
	}
	return Results;
}
void UDamageCalculator::ProcessStatistics(const FCombatHitResult& Result, bool bIsAttackerTeam)
{
	FTeamCombatStats& Stats = bIsAttackerTeam ? AttackerStatistic : DefenderStatistic;
	if (Result.bHit)
	{
		Stats.Hits++;
		Stats.TotalDamage += Result.Damage;
	}
	else
	{
		Stats.Misses++;
	}
	Stats.EffectsApplied += Result.EffectsApplied.Num();
}
void UDamageCalculator::ResetStatistics()
{
	AttackerStatistic.Reset();
	DefenderStatistic.Reset();
}
bool UDamageCalculator::PerformAccuracyRoll(float HitChance)
{
	float Roll = FMath::FRand() * 100.0f;
	return Roll <= HitChance;
}
EDamageSource UDamageCalculator::SelectBestDamageSource(const TSet<EDamageSource>& DamageSources, AUnit* Target)
{
	if (!Target || DamageSources.Num() == 0)
	{
		return EDamageSource::None;
	}
	const FUnitDefenseStats& Defense = Target->GetStats().Defense;
	EDamageSource BestSource = EDamageSource::None;
	float LowestArmor = 1.0f;
	bool bFirstSource = true;
	for (EDamageSource Source : DamageSources)
	{
		float Armor = 0.0f;
		if (Defense.Armour.Contains(Source))
		{
			Armor = Defense.Armour[Source];
		}
		if (bFirstSource || Armor < LowestArmor)
		{
			LowestArmor = Armor;
			BestSource = Source;
			bFirstSource = false;
		}
	}
	return BestSource;
}

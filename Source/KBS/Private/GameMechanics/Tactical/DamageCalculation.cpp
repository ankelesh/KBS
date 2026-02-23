#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Stats/UnitStats.h"


float FDamageCalculation::CalculateHitChance(AUnit* Attacker, UWeapon* Weapon, AUnit* Target)
{
	if (!Attacker || !Weapon || !Target)
	{
		return 0.0f;
	}
	const FUnitCoreStats& AttackerStats = Attacker->GetStats();
	const FWeaponStats& WeaponStats = Weapon->GetStats();
	float HitChance = AttackerStats.Accuracy.GetValue() * WeaponStats.AccuracyMultiplier / 100.0f;
	return FMath::Clamp(HitChance, 0.0f, 100.0f);
}

FDamageResult FDamageCalculation::CalculateDamage(AUnit* Attacker, UWeapon* Weapon, AUnit* Target)
{
	FDamageResult Result;
	if (!Attacker || !Weapon || !Target)
	{
		return Result;
	}
	const FWeaponStats& WeaponStats = Weapon->GetStats();
	const FUnitCoreStats& TargetStats = Target->GetStats();
	const FUnitDefenseStats& Defense = TargetStats.Defense;
	const bool bIsTargetDefending = Target->GetStats().Status.IsDefending(); 
	EDamageSource BestSource = SelectBestDamageSource(WeaponStats.DamageSources.GetValue(), Target);
	Result.DamageSource = BestSource;
	if (BestSource == EDamageSource::None)
	{
		return Result;
	}
	int32 BaseDamage = WeaponStats.BaseDamage.GetValue();
	if (Defense.Immunities.IsImmuneTo(BestSource))
	{
		Result.Damage = 0;
		Result.DamageBlocked = BaseDamage;
		return Result;
	}
	if (Defense.Wards.HasWardFor(BestSource))
	{
		Result.Damage = 0;
		Result.DamageBlocked = BaseDamage;
		Result.WardSpent = BestSource;
		return Result;
	}
	int32 ArmorPercent = Defense.Armour.GetValue(BestSource);
	float ArmorValue = ArmorPercent / 100.0f;
	float DamageAfterArmor = BaseDamage * (1.0f - ArmorValue);
	float FinalDamage = DamageAfterArmor - Defense.DamageReduction;
	if (bIsTargetDefending)
	{
		FinalDamage *= 0.5f;
	}
	Result.Damage = FMath::RoundToInt(FinalDamage);
	Result.DamageBlocked = BaseDamage - Result.Damage;
	return Result;
}

float FDamageCalculation::CalculateEffectApplication(AUnit* Attacker, UBattleEffect* Effect, AUnit* Target)
{
	if (!Attacker || !Effect || !Target)
	{
		return 0.0f;
	}
	const FUnitCoreStats& AttackerStats = Attacker->GetStats();
	const FUnitDefenseStats& Defense = Target->GetStats().Defense;
	EDamageSource EffectSource = Effect->GetDamageSource();
	if (Defense.Immunities.IsImmuneTo(EffectSource))
	{
		return 0.0f;
	}
	if (Defense.Wards.HasWardFor(EffectSource))
	{
		return 0.0f;
	}
	float ApplicationChance = AttackerStats.Accuracy.GetValue();
	return FMath::Clamp(ApplicationChance, 0.0f, 100.0f);
}

UWeapon* FDamageCalculation::SelectMaxReachWeapon(AUnit* Unit, bool bAutoAttackOnly)
{
	if (!Unit)
	{
		return nullptr;
	}
	auto Weapons = Unit->GetWeapons();
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
		if (bAutoAttackOnly && !Weapon->IsUsableForAutoAttack()) continue;
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

UWeapon* FDamageCalculation::SelectSpellWeapon(AUnit* Unit)
{
	if (!Unit) return nullptr;
	for (UWeapon* Weapon : Unit->GetWeapons())
	{
		if (Weapon && Weapon->IsUsableForSpells())
		{
			return Weapon;
		}
	}
	return nullptr;
}

void FDamageCalculation::ApplySpellScaling(UWeapon* EmbeddedWeapon, UWeapon* SpellWeapon, float Multiplier, int32 FlatBonus)
{
	int32 SpellDamage = SpellWeapon->GetStats().BaseDamage.GetValue();
	int32 NewBase = FMath::Max(0, FMath::RoundToInt(SpellDamage * Multiplier) + FlatBonus);
	EmbeddedWeapon->GetMutableStats().BaseDamage.SetBase(NewBase);
}

FPreviewHitResult FDamageCalculation::PreviewDamage(AUnit* Attacker, UWeapon* Weapon, AUnit* Target)
{
	FPreviewHitResult Preview;
	if (!Attacker || !Target || !Weapon)
	{
		return Preview;
	}
	Preview.HitProbability = CalculateHitChance(Attacker, Weapon, Target);
	Preview.DamageResult = CalculateDamage(Attacker, Weapon, Target);
	if (const TArray<UBattleEffect*>& Effects = Weapon->GetEffects(); Effects.Num() > 0)
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

bool FDamageCalculation::PerformAccuracyRoll(float HitChance)
{
	float Roll = FMath::FRand() * 100.0f;
	return Roll <= HitChance;
}

bool FDamageCalculation::IsFriendlyReach(ETargetReach Reach)
{
	return Reach == ETargetReach::Self ||
		Reach == ETargetReach::AnyFriendly ||
		Reach == ETargetReach::AllFriendlies ||
		Reach == ETargetReach::EmptyCellOrFriendly ||
		Reach == ETargetReach::FriendlyCorpse ||
		Reach == ETargetReach::FriendlyNonBlockedCorpse;
}

EDamageSource FDamageCalculation::SelectBestDamageSource(const TSet<EDamageSource>& DamageSources, AUnit* Target)
{
	if (!Target || DamageSources.Num() == 0)
	{
		return EDamageSource::None;
	}
	const FUnitDefenseStats& Defense = Target->GetStats().Defense;
	EDamageSource BestSource = EDamageSource::None;
	int32 LowestArmor = 100;
	bool bFirstSource = true;
	for (EDamageSource Source : DamageSources)
	{
		int32 Armor = Defense.Armour.GetValue(Source);
		if (bFirstSource || Armor < LowestArmor)
		{
			LowestArmor = Armor;
			BestSource = Source;
			bFirstSource = false;
		}
	}
	return BestSource;
}


#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Stats/UnitStats.h"


float FDamageCalculation::CalculateHitChance(AUnit* Attacker, UCombatDescriptor* Descriptor, AUnit* Target)
{
	if (!Attacker || !Descriptor || !Target)
	{
		return 0.0f;
	}
	const FUnitCoreStats& AttackerStats = Attacker->GetStats();
	const FCombatDescriptorStats& DescriptorStats = Descriptor->GetStats();
	float HitChance = AttackerStats.Accuracy.GetValue() * DescriptorStats.AccuracyMultiplier / 100.0f;
	return FMath::Clamp(HitChance, 0.0f, 100.0f);
}

FDamageResult FDamageCalculation::CalculateDamage(AUnit* Attacker, UCombatDescriptor* Descriptor, AUnit* Target)
{
	FDamageResult Result;
	if (!Attacker || !Descriptor || !Target)
	{
		return Result;
	}
	const FCombatDescriptorStats& DescriptorStats = Descriptor->GetStats();
	const FUnitCoreStats& TargetStats = Target->GetStats();
	const FUnitDefenseStats& Defense = TargetStats.Defense;
	const bool bIsTargetDefending = Target->GetStats().Status.IsDefending();
	EDamageSource BestSource = SelectBestDamageSource(DescriptorStats.DamageSources.GetValue(), Target);
	Result.DamageSource = BestSource;
	if (BestSource == EDamageSource::None)
	{
		return Result;
	}
	int32 BaseMagnitude = DescriptorStats.BaseMagnitude.GetValue();
	if (Defense.Immunities.IsImmuneTo(BestSource))
	{
		Result.Damage = 0;
		Result.DamageBlocked = BaseMagnitude;
		return Result;
	}
	if (Defense.Wards.HasWardFor(BestSource))
	{
		Result.Damage = 0;
		Result.DamageBlocked = BaseMagnitude;
		Result.WardSpent = BestSource;
		return Result;
	}
	int32 ArmorPercent = Defense.Armour.GetValue(BestSource);
	float ArmorValue = ArmorPercent / 100.0f;
	float DamageAfterArmor = BaseMagnitude * (1.0f - ArmorValue);
	if (Attacker->GetGridMetadata().bOnFlank && !Attacker->GetStats().Status.IsFlankDelayed())
	{
		DamageAfterArmor *= FLANKING_DAMAGE_MULTIPLIER;
		BaseMagnitude *= FLANKING_DAMAGE_MULTIPLIER;
	}
	float FinalDamage = DamageAfterArmor - Defense.DamageReduction;
	if (bIsTargetDefending)
	{
		FinalDamage *= DEFENSIVE_STANCE_MULTIPLIER;
	}
	Result.Damage = FMath::RoundToInt(FinalDamage);
	Result.DamageBlocked = BaseMagnitude - Result.Damage;
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

FDamageResult FDamageCalculation::CalculateHeal(AUnit* Attacker, UCombatDescriptor* Descriptor, AUnit* Target)
{
	if (!Attacker || !Descriptor || !Target) return FDamageResult();

	const FUnitDefenseStats& Defense = Target->GetStats().Defense;
	const TSet<EDamageSource>& Sources = Descriptor->GetStats().DamageSources.GetValue();

	EDamageSource BestSource = EDamageSource::None;
	EDamageSource ImmunityFallback = EDamageSource::None;
	EDamageSource WardFallback = EDamageSource::None;

	for (EDamageSource Source : Sources)
	{
		const bool bImmune = Defense.Immunities.IsImmuneTo(Source);
		const bool bWarded = Defense.Wards.HasWardFor(Source);
		if (!bImmune && !bWarded)
		{
			BestSource = Source;
			break;
		}
		if (bImmune && ImmunityFallback == EDamageSource::None)
			ImmunityFallback = Source;
		else if (bWarded && WardFallback == EDamageSource::None)
			WardFallback = Source;
	}

	if (BestSource == EDamageSource::None)
		BestSource = (ImmunityFallback != EDamageSource::None) ? ImmunityFallback : WardFallback;

	FDamageResult Result;
	Result.DamageSource = BestSource;
	Result.Damage = Descriptor->GetStats().BaseMagnitude.GetValue();
	return Result;
}

UCombatDescriptor* FDamageCalculation::SelectMaxReachDescriptor(AUnit* Unit, bool bAutoAttackOnly)
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
	UCombatDescriptor* BestDescriptor = nullptr;
	int32 BestScore = -1;
	for (UCombatDescriptor* Descriptor : Weapons)
	{
		if (bAutoAttackOnly && !Descriptor->IsUsableForAutoAttack()) continue;
		const FCombatDescriptorStats& Stats = Descriptor->GetStats();
		int32 Score = GetReachScore(Stats.TargetReach);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestDescriptor = Descriptor;
		}
	}
	return BestDescriptor;
}

UCombatDescriptor* FDamageCalculation::SelectSpellDescriptor(AUnit* Unit)
{
	if (!Unit) return nullptr;
	for (UCombatDescriptor* Descriptor : Unit->GetWeapons())
	{
		if (Descriptor && Descriptor->IsUsableForSpells())
		{
			return Descriptor;
		}
	}
	return nullptr;
}

void FDamageCalculation::ApplySpellScaling(UCombatDescriptor* EmbeddedDescriptor, UCombatDescriptor* SpellDescriptor, float Multiplier, int32 FlatBonus)
{
	int32 SpellDamage = SpellDescriptor->GetStats().BaseMagnitude.GetValue();
	int32 NewBase = FMath::Max(0, FMath::RoundToInt(SpellDamage * Multiplier) + FlatBonus);
	EmbeddedDescriptor->SetMagnitudeBase(NewBase);
}

FPreviewHitResult FDamageCalculation::PreviewDamage(AUnit* Attacker, UCombatDescriptor* Descriptor, AUnit* Target)
{
	FPreviewHitResult Preview;
	if (!Attacker || !Target || !Descriptor)
	{
		return Preview;
	}
	Preview.HitProbability = CalculateHitChance(Attacker, Descriptor, Target);
	Preview.DamageResult = CalculateDamage(Attacker, Descriptor, Target);
	if (const TArray<UBattleEffect*>& Effects = Descriptor->GetEffects(); Effects.Num() > 0)
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

UCombatDescriptor* FDamageCalculation::SelectDescriptorForTarget(AUnit* Attacker, AUnit* Target, bool bAutoAttackOnly)
{
	check(Attacker && Target);

	const TArray<UCombatDescriptor*>& Weapons = Attacker->GetWeapons();
	if (Weapons.Num() == 0) return nullptr;

	const int32 Distance = Attacker->GetGridMetadata().DistanceTo(Target->GetGridMetadata());
	const bool bIsFriendly = Attacker->GetGridMetadata().IsAlly(Target->GetGridMetadata());

	// Returns whether a descriptor with this reach can hit a target at Distance with the given affiliation.
	// Collapses all reach variants to two axes: affiliation (friendly/hostile) and range (melee/any).
	auto CanReachTarget = [&](ETargetReach Reach) -> bool
	{
		switch (Reach)
		{
		// Hostile melee-only
		case ETargetReach::ClosestEnemies:
			return !bIsFriendly && Distance == 1;

		// Hostile any distance
		case ETargetReach::AnyEnemy:
		case ETargetReach::AllEnemies:
		case ETargetReach::Area:
		case ETargetReach::AreaEnemy:
			return !bIsFriendly;

		// Friendly any distance
		case ETargetReach::Self:
		case ETargetReach::AnyFriendly:
		case ETargetReach::AllFriendlies:
		case ETargetReach::EmptyCellOrFriendly:
		case ETargetReach::AreaFriendly:
			return bIsFriendly;

		// Non-unit targets (corpse, movement, empty cell) — never targets a live unit
		default:
			return false;
		}
	};

	UCombatDescriptor* BestDescriptor = nullptr;
	int32 BestDamage = -1;
	for (UCombatDescriptor* Descriptor : Weapons)
	{
		if (bAutoAttackOnly && !Descriptor->IsUsableForAutoAttack()) continue;
		if (!CanReachTarget(Descriptor->GetStats().TargetReach)) continue;

		const int32 Dmg = CalculateDamage(Attacker, Descriptor, Target).Damage;
		if (Dmg > BestDamage)
		{
			BestDamage = Dmg;
			BestDescriptor = Descriptor;
		}
	}
	return BestDescriptor;
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


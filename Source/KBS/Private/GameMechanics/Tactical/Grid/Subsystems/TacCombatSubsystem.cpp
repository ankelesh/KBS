// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacCombatStatisticsService.h"
#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacAbilityExecutorService.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"

TArray<FCombatHitResult> UTacCombatSubsystem::ResolveAttack(AUnit* Attacker, TArray<AUnit*> Targets, UWeapon* Weapon)
{
	TArray<FCombatHitResult> Results;
	if (!Attacker || !Weapon || Targets.Num() == 0)
	{
		return Results;
	}

	FAttackContext Context(Attacker, Weapon, Targets, false);
	if (!Context.Attacker || !Context.AttackerWeapon || Context.Hits.Num() == 0)
	{
		return Results;
	}
	return ResolveAttackInternal(Context);
}

TArray<FCombatHitResult> UTacCombatSubsystem::ResolveReactionAttack(AUnit* Attacker, TArray<AUnit*> Targets, UWeapon* Weapon)
{
	TArray<FCombatHitResult> Results;
	if (!Attacker || !Weapon || Targets.Num() == 0)
	{
		return Results;
	}

	FAttackContext Context(Attacker, Weapon, Targets, true);
	if (!Context.Attacker || !Context.AttackerWeapon || Context.Hits.Num() == 0)
	{
		return Results;
	}
	return ResolveAttackInternal(Context);
}

TArray<FCombatHitResult> UTacCombatSubsystem::ResolveAttackInternal(FAttackContext& Context)
{
	TArray<FCombatHitResult> Results;

	if (!ExecutePreAttackPhase(Context))
	{
		for (const FHitInstance& Hit : Context.Hits)
		{
			Results.Add(FCombatHitResult::Miss(Hit.Target));
		}
		return Results;
	}

	for (FHitInstance& Hit : Context.Hits)
	{
		FCombatHitResult Result;
		ExecuteCalculationPhase(Context, Hit, Result);
		if (!Result.bHit)
		{
			Results.Add(Result);
			continue;
		}
		ExecuteDamageApplyPhase(Context, Hit, Result);
		if (Result.bHit)
		{
			ExecuteEffectApplicationPhase(Context, Hit, Result);
		}
		Results.Add(Result);
	}
	return Results;
}

bool UTacCombatSubsystem::ExecutePreAttackPhase(FAttackContext& Context)
{
	OnPreUnitAttackPhase.Broadcast(Context);
	Context.CheckCancellation();
	if (Context.bIsAttackCancelled)
	{
		return false;
	}

	for (FHitInstance& Hit : Context.Hits)
	{
		Hit.Target->OnUnitAttacked.Broadcast(Hit.Target, Context.Attacker);
		Context.Attacker->OnUnitAttacks.Broadcast(Context.Attacker, Hit.Target);
		Hit.CheckCancellation();
	}

	return true;
}

void UTacCombatSubsystem::ExecuteCalculationPhase(FAttackContext& Context, FHitInstance& Hit, FCombatHitResult& OutResult)
{
	OnCalculationPhase.Broadcast(Context, Hit);
	Hit.CheckCancellation();
	if (Hit.bIsHitCancelled || Context.bIsAttackCancelled)
	{
		OutResult = FCombatHitResult::Miss(Hit.Target);
		return;
	}

	OutResult.TargetUnit = Hit.Target;
	if (FDamageCalculation::IsFriendlyReach(Context.AttackerWeapon->GetReach()))
	{
		OutResult.bHit = true;
	}
	else
	{
		OutResult.bHit = FDamageCalculation::PerformAccuracyRoll(
			FDamageCalculation::CalculateHitChance(Context.Attacker, Context.AttackerWeapon, Hit.Target));
	}

	if (OutResult.bHit)
	{
		OutResult.DamageResult = FDamageCalculation::CalculateDamage(Context.Attacker, Context.AttackerWeapon, Hit.Target);
	}
}

void UTacCombatSubsystem::ExecuteEffectApplicationPhase(FAttackContext& Context, FHitInstance& Hit, FCombatHitResult& Result)
{
	OnEffectApplicationPhase.Broadcast(Context, Hit);
	Hit.CheckCancellation();
	if (Hit.bIsHitCancelled || Context.bIsAttackCancelled)
	{
		return;
	}

	for (UBattleEffect* Effect : Context.AttackerWeapon->GetEffects())
	{
		if (Effect->IsRequringRoll())
		{
			if (!FDamageCalculation::PerformAccuracyRoll(
				FDamageCalculation::CalculateEffectApplication(Context.Attacker, Effect, Hit.Target)))
			{
				continue;
			}
		}
		TObjectPtr<UBattleEffect> EffectCopy = DuplicateObject(Effect, Hit.Target);
		EffectCopy->PrepareForApply(Context.Attacker, Hit.Target);
		if (Hit.Target->ApplyEffect(EffectCopy))
		{
			++Result.EffectsApplied;
		}
	}
}

void UTacCombatSubsystem::ExecuteDamageApplyPhase(FAttackContext& Context, FHitInstance& Hit, FCombatHitResult& ToApply)
{
	OnDamageApplicationPhase.Broadcast(Context, Hit, ToApply.DamageResult);
	Hit.CheckCancellation();
	if (Hit.bIsHitCancelled || Context.bIsAttackCancelled)
	{
		ToApply.bHit = false;
		ToApply.DamageResult.DamageBlocked += ToApply.DamageResult.Damage;
		ToApply.DamageResult.Damage = 0;
	}
	else
	{
		Hit.Target->TakeHit(ToApply.DamageResult);
	}
}

void UTacCombatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	AbilityExecutorService = NewObject<UTacAbilityExecutorService>(this);
	CombatStatisticsService = NewObject<UTacCombatStatisticsService>(this);
}

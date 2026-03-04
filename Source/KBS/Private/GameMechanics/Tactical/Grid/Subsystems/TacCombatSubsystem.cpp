// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
DEFINE_LOG_CATEGORY(LogKBSCombat);
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacCombatStatisticsService.h"
#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacAbilityExecutorService.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"

namespace
{
	void LogCancellation(const FCombatContext& Context, const TCHAR* Phase)
	{
		UE_LOG(LogKBSCombat, Log, TEXT("[%s CANCELLED] %s's action was cancelled"), Phase, *Context.Attacker->GetName());
	}

	void LogCancellation(const FCombatContext& Context, const FHitInstance& Hit, const TCHAR* Phase)
	{
		UE_LOG(LogKBSCombat, Log, TEXT("[%s CANCELLED] %s -> %s"), Phase, *Context.Attacker->GetName(), *Hit.Target->GetName());
	}
}

TArray<FCombatHitResult> UTacCombatSubsystem::ResolveAttack(AUnit* Attacker, TArray<AUnit*> Targets, UWeapon* Weapon)
{
	TArray<FCombatHitResult> Results;
	if (!Attacker || !Weapon || Targets.Num() == 0)
	{
		return Results;
	}

	FCombatContext Context(Attacker, Weapon, Targets, false);
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

	FCombatContext Context(Attacker, Weapon, Targets, true);
	if (!Context.Attacker || !Context.AttackerWeapon || Context.Hits.Num() == 0)
	{
		return Results;
	}
	return ResolveAttackInternal(Context);
}

TArray<FCombatHitResult> UTacCombatSubsystem::ResolveAttackInternal(FCombatContext& Context)
{
	TArray<FCombatHitResult> Results;
	LogAttackStart(Context);
	if (!ExecutePreResolutionPhase(Context))
	{
		for (const FHitInstance& Hit : Context.Hits)
		{
			Results.Add(FCombatHitResult::Cancelled(Hit.Target));
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
		ExecuteResultApplyPhase(Context, Hit, Result);
		if (Result.bHit)
		{
			ExecuteEffectApplicationPhase(Context, Hit, Result);
		}
		Results.Add(Result);
	}

	int32 HitCount = 0, TotalDamage = 0;
	for (const FCombatHitResult& R : Results) { if (R.bHit) { ++HitCount; TotalDamage += R.DamageResult.Damage; } }
	UE_LOG(LogKBSCombat, Log, TEXT("[ATTACK END] %s: %d/%d hit, total_dmg=%d"),
		*Context.Attacker->GetLogName(), HitCount, Results.Num(), TotalDamage);

	return Results;
}



bool UTacCombatSubsystem::ExecutePreResolutionPhase(FCombatContext& Context)
{
	OnPreResolutionPhase.Broadcast(Context);
	Context.CheckCancellation();
	if (Context.bIsAttackCancelled)
	{
		LogCancellation(Context, TEXT("PRE-RESOLUTION"));
		return false;
	}

	Context.Attacker->EnterCombatResolutionPhase(Context, Context.Hits[0], false);
	for (FHitInstance& Hit : Context.Hits)
	{
		Hit.Target->OnUnitAttacked.Broadcast(Hit.Target, Context.Attacker);
		Context.Attacker->OnUnitAttacks.Broadcast(Context.Attacker, Hit.Target);
		Hit.Target->EnterCombatResolutionPhase(Context, Hit, true);
		Hit.CheckCancellation();
	}

	return true;
}

void UTacCombatSubsystem::ExecuteCalculationPhase(FCombatContext& Context, FHitInstance& Hit, FCombatHitResult& OutResult)
{
	OnCalculationPhase.Broadcast(Context, Hit);
	Context.Attacker->EnterCombatCalculationPhase(Context, Hit, false);
	Hit.Target->EnterCombatCalculationPhase(Context, Hit, true);
	Hit.CheckCancellation();
	if (Hit.bIsHitCancelled || Context.bIsAttackCancelled)
	{
		LogCancellation(Context, Hit, TEXT("HIT"));
		OutResult = FCombatHitResult::Cancelled(Hit.Target);
		return;
	}

	OutResult.TargetUnit = Hit.Target;
	if (Context.AttackerWeapon->IsRequiringAccuracyRoll())
	{
		OutResult.bHit = FDamageCalculation::PerformAccuracyRoll(
			FDamageCalculation::CalculateHitChance(Context.Attacker, Context.AttackerWeapon, Hit.Target));
	}
	else
	{
		OutResult.bHit = true;
	}

	if (OutResult.bHit && Context.AttackerWeapon->GetIntent() == EAttackIntent::Attack)
	{
		OutResult.DamageResult = FDamageCalculation::CalculateDamage(Context.Attacker, Context.AttackerWeapon, Hit.Target);
	}
}

void UTacCombatSubsystem::ExecuteEffectApplicationPhase(FCombatContext& Context, FHitInstance& Hit, FCombatHitResult& Result)
{
	OnEffectApplicationPhase.Broadcast(Context, Hit);
	Context.Attacker->EnterCombatEffectApplicationPhase(Context, Hit, false);
	Hit.Target->EnterCombatEffectApplicationPhase(Context, Hit, true);
	Hit.CheckCancellation();
	if (Hit.bIsHitCancelled || Context.bIsAttackCancelled)
	{
		LogCancellation(Context, Hit, TEXT("EFFECTS"));
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

void UTacCombatSubsystem::ExecuteResultApplyPhase(FCombatContext& Context, FHitInstance& Hit, FCombatHitResult& ToApply)
{
	OnResultApplicationPhase.Broadcast(Context, Hit, ToApply.DamageResult);
	Context.Attacker->EnterCombatResultApplyPhase(Context, Hit, ToApply.DamageResult, false);
	Hit.Target->EnterCombatResultApplyPhase(Context, Hit, ToApply.DamageResult, true);
	Hit.CheckCancellation();
	if (Hit.bIsHitCancelled || Context.bIsAttackCancelled)
	{
		LogCancellation(Context, Hit, TEXT("DAMAGE"));
		ToApply.bHit = false;
		ToApply.DamageResult.DamageBlocked += ToApply.DamageResult.Damage;
		ToApply.DamageResult.Damage = 0;
	}
	else
	{
		if (Context.Intent == EAttackIntent::Attack)
			Hit.Target->HandleHit(ToApply.DamageResult, Context.Attacker);
		else if (Context.Intent == EAttackIntent::Heal)
		{
			Hit.Target->ChangeUnitHP(ToApply.DamageResult.Damage);
		}
	}
}

void UTacCombatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	AbilityExecutorService = NewObject<UTacAbilityExecutorService>(this);
	CombatStatisticsService = NewObject<UTacCombatStatisticsService>(this);
	CombatLogger = MakeUnique<FTacCategoryLogger>(FName("LogKBSCombat"), TEXT("Combat"));
}




void UTacCombatSubsystem::LogAttackStart(FCombatContext& Context)
{
	TArray<FString> TargetNames;
	for (const FHitInstance& Hit : Context.Hits) { TargetNames.Add(Hit.Target->GetName()); }
	const UWeaponDataAsset* WeaponConfig = Context.AttackerWeapon ? Context.AttackerWeapon->GetConfig() : nullptr;
	UE_LOG(LogKBSCombat, Log, TEXT("[ATTACK START] %s -> [%s] weapon=%s reaction=%d"),
		*Context.Attacker->GetLogName(), *FString::Join(TargetNames, TEXT(", ")),
		WeaponConfig ? *WeaponConfig->GetName() : TEXT("none"), Context.bIsReactionHit ? 1 : 0);
}

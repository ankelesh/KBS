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

	TArray<FString> TargetNames;
	for (const FHitInstance& Hit : Context.Hits) { TargetNames.Add(Hit.Target->GetName()); }
	const UWeaponDataAsset* WeaponConfig = Context.AttackerWeapon ? Context.AttackerWeapon->GetConfig() : nullptr;
	UE_LOG(LogKBSCombat, Log, TEXT("[ATTACK START] %s -> [%s] weapon=%s reaction=%d"),
		*Context.Attacker->GetName(), *FString::Join(TargetNames, TEXT(", ")),
		WeaponConfig ? *WeaponConfig->GetName() : TEXT("none"), Context.bIsReactionHit ? 1 : 0);

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

	int32 HitCount = 0, TotalDamage = 0;
	for (const FCombatHitResult& R : Results) { if (R.bHit) { ++HitCount; TotalDamage += R.DamageResult.Damage; } }
	UE_LOG(LogKBSCombat, Log, TEXT("[ATTACK END] %s: %d/%d hit, total_dmg=%d"),
		*Context.Attacker->GetName(), HitCount, Results.Num(), TotalDamage);

	return Results;
}

bool UTacCombatSubsystem::ExecutePreAttackPhase(FAttackContext& Context)
{
	OnPreUnitAttackPhase.Broadcast(Context);
	Context.CheckCancellation();
	if (Context.bIsAttackCancelled)
	{
		UE_LOG(LogKBSCombat, Log, TEXT("[PRE-ATTACK CANCELLED] %s's attack was cancelled"), *Context.Attacker->GetName());
		return false;
	}

	Context.Attacker->OnStartingAttack.Broadcast(Context, Context.Hits[0]);
	for (FHitInstance& Hit : Context.Hits)
	{
		Hit.Target->OnUnitAttacked.Broadcast(Hit.Target, Context.Attacker);
		Context.Attacker->OnUnitAttacks.Broadcast(Context.Attacker, Hit.Target);
		Hit.Target->OnBeingAttackedPrePhase.Broadcast(Context, Hit);
		Hit.CheckCancellation();
	}

	return true;
}

void UTacCombatSubsystem::ExecuteCalculationPhase(FAttackContext& Context, FHitInstance& Hit, FCombatHitResult& OutResult)
{
	OnCalculationPhase.Broadcast(Context, Hit);
	Context.Attacker->OnAttackingInCalculation.Broadcast(Context, Hit);
	Hit.Target->OnBeingTargetedInCalculation.Broadcast(Context, Hit);
	Hit.CheckCancellation();
	if (Hit.bIsHitCancelled || Context.bIsAttackCancelled)
	{
		UE_LOG(LogKBSCombat, Log, TEXT("[HIT CANCELLED] %s -> %s: cancelled in calculation phase"),
			*Context.Attacker->GetName(), *Hit.Target->GetName());
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
		UE_LOG(LogKBSCombat, Log, TEXT("[HIT] %s -> %s: dmg=%d blocked=%d"),
			*Context.Attacker->GetName(), *Hit.Target->GetName(),
			OutResult.DamageResult.Damage, OutResult.DamageResult.DamageBlocked);
	}
	else
	{
		UE_LOG(LogKBSCombat, Log, TEXT("[MISS] %s -> %s"), *Context.Attacker->GetName(), *Hit.Target->GetName());
	}
}

void UTacCombatSubsystem::ExecuteEffectApplicationPhase(FAttackContext& Context, FHitInstance& Hit, FCombatHitResult& Result)
{
	OnEffectApplicationPhase.Broadcast(Context, Hit);
	Context.Attacker->OnAttackingEffectApplication.Broadcast(Context, Hit);
	Hit.Target->OnBeingTargetedForEffects.Broadcast(Context, Hit);
	Hit.CheckCancellation();
	if (Hit.bIsHitCancelled || Context.bIsAttackCancelled)
	{
		UE_LOG(LogKBSCombat, Log, TEXT("[EFFECTS CANCELLED] %s -> %s: effect phase cancelled"),
			*Context.Attacker->GetName(), *Hit.Target->GetName());
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

	if (Result.EffectsApplied > 0)
	{
		UE_LOG(LogKBSCombat, Log, TEXT("[EFFECTS] %d effects applied to %s"), Result.EffectsApplied, *Hit.Target->GetName());
	}
}

void UTacCombatSubsystem::ExecuteDamageApplyPhase(FAttackContext& Context, FHitInstance& Hit, FCombatHitResult& ToApply)
{
	OnDamageApplicationPhase.Broadcast(Context, Hit, ToApply.DamageResult);
	Context.Attacker->OnAttackingDamageApply.Broadcast(Context, Hit, ToApply.DamageResult);
	Hit.Target->OnBeingHitInDamageApply.Broadcast(Context, Hit, ToApply.DamageResult);
	Hit.CheckCancellation();
	if (Hit.bIsHitCancelled || Context.bIsAttackCancelled)
	{
		UE_LOG(LogKBSCombat, Log, TEXT("[DAMAGE BLOCKED] %s -> %s: hit cancelled at damage phase"),
			*Context.Attacker->GetName(), *Hit.Target->GetName());
		ToApply.bHit = false;
		ToApply.DamageResult.DamageBlocked += ToApply.DamageResult.Damage;
		ToApply.DamageResult.Damage = 0;
	}
	else
	{
		Hit.Target->HandleHit(ToApply.DamageResult, Context.Attacker);
	}
}

void UTacCombatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	AbilityExecutorService = NewObject<UTacAbilityExecutorService>(this);
	CombatStatisticsService = NewObject<UTacCombatStatisticsService>(this);
	CombatLogger = MakeUnique<FTacCategoryLogger>(FName("LogKBSCombat"), TEXT("Combat"));
}

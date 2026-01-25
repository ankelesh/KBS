// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameMechanics/Units/UnitStats.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "CombatMutationService.generated.h"

UENUM()
enum EMutationFailureReason
{
	None,
	Blocked,
	Missed,
	Intercepted,
	Capped
};

USTRUCT()
struct FEffectApplyResult {
	GENERATED_BODY()
	bool bSuccess = true;
	EMutationFailureReason FailureReason = None;
	TObjectPtr<UBattleEffect> Effect = nullptr;
};

USTRUCT()
struct FMutationResult {
	GENERATED_BODY()
	bool bSuccess = true;
	EMutationFailureReason FailureReason = None;
	int32 HealthDelta = 0;
	TOptional<FUnitCoreStats> StatDelta;
	TArray<TObjectPtr<UBattleEffect>> AppliedEffects;
};

typedef TArray<FMutationResult> FMutationResults;
typedef TArray<FEffectApplyResult> FEffectApplyResults;
UCLASS()
class KBS_API UCombatMutationService : public UObject
{
	GENERATED_BODY()

public:
	FMutationResult ApplyHealthMutation(AUnit* Source, AUnit* Target, int32 Delta);
	FMutationResult ApplyStatMutation(AUnit* Source, AUnit* Target, const FUnitCoreStats& delta);
	FMutationResult RemoveStatMutation(AUnit* Source, AUnit* Target, const FUnitCoreStats& delta);
	FMutationResult ResetUnitStats(AUnit* Source, AUnit* Target);
	FMutationResult ApplyPureDamage(AUnit* Source, AUnit* Target, int32 Damage);
	FMutationResult SetDefensiveStance(AUnit* Target, bool bDefensiveStance);
	FEffectApplyResult ApplyEffect(AUnit* Source, AUnit* Target, TObjectPtr<UBattleEffect> Effect);
	FEffectApplyResult RemoveEffect(AUnit* Source, AUnit* Target, TObjectPtr<UBattleEffect> Effect);
	FEffectApplyResult ApplyEffects(AUnit* Source, AUnit* Target, BattleEffectArray Effects);
};
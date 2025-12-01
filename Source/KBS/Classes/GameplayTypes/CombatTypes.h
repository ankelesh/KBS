// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "CombatTypes.generated.h"

class AUnit;
class UBattleEffect;

USTRUCT(BlueprintType)
struct FDamageResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Damage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DamageBlocked = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageSource DamageSource = EDamageSource::Physical;
};

USTRUCT(BlueprintType)
struct FHitInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AUnit> SourceUnit = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<AUnit>> TargetUnits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETargetReach SelectedRange = ETargetReach::AnyEnemy;
};

USTRUCT(BlueprintType)
struct FCombatHitResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Damage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageSource DamageSource = EDamageSource::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageSource WardSpent = EDamageSource::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DamageBlocked = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UBattleEffect>> EffectsApplied;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AUnit> TargetUnit = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHit = false;
};

USTRUCT(BlueprintType)
struct FPreviewHitResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitProbability = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDamageResult DamageResult;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectApplicationProbability = 0.0f;
};

USTRUCT(BlueprintType)
struct FTeamCombatStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalDamage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Hits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Misses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EffectsApplied = 0;

	void Reset()
	{
		TotalDamage = 0;
		Hits = 0;
		Misses = 0;
		EffectsApplied = 0;
	}
};

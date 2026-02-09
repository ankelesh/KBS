#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Stats/UnitStats.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageSource WardSpent = EDamageSource::None;
};

USTRUCT(BlueprintType)
struct FHitInstance
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AUnit* Attacker = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AUnit* Target = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHitCancelled = false;
	UPROPERTY()
	TArray<UUnitAbilityInstance*> InterferingAbilities;
	
	void Reset();
	explicit FHitInstance(AUnit* TargetUnit, AUnit* AttackerUnit, UWeapon* Weapon);
	void Interfere(UUnitAbilityInstance* Ability, bool bIsCancelled = false);
	void CheckCancellation();
	~FHitInstance();
};

USTRUCT(BlueprintType)
struct FAttackContext
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AUnit* Attacker;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FHitInstance> Hits;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWeapon* AttackerWeapon;
	UPROPERTY()
	TArray<UUnitAbilityInstance*> InterferingAbilities;
	UPROPERTY()
	bool bIsAttackCancelled;
	UPROPERTY()
	bool bIsReactionHit;
	
	void Reset();
	explicit FAttackContext(AUnit* AttackerUnit, UWeapon* Weapon, TArray<AUnit*> Targets, bool bIsReaction = false);
	void Interfere(UUnitAbilityInstance* Ability, bool bIsCancelled = false);
	void CheckCancellation();
	~FAttackContext();
};



USTRUCT(BlueprintType)
struct FCombatHitResult
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDamageResult DamageResult;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EffectsApplied = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AUnit> TargetUnit = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHit = false;

	static FCombatHitResult NoHitHappened(AUnit* Target)
	{
		FCombatHitResult Res;
		Res.bHit = false;
		Res.TargetUnit = Target;
		return Res;
	}
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
struct FResolvedTargets
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AUnit> ClickedTarget = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<AUnit>> SecondaryTargets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasCellEmpty = false;

	TArray<AUnit*> GetAllTargets() const
	{
		TArray<AUnit*> AllTargets;
		if (ClickedTarget)
		{
			AllTargets.Add(ClickedTarget);
		}
		AllTargets.Append(SecondaryTargets);
		return AllTargets;
	}
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

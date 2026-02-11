// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTypes/CombatTypes.h"
#include "TacCombatSubsystem.generated.h"

class UGridDataManager;
class UTacAbilityExecutorService;
class UTacCombatStatisticsService;
class AUnit;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreUnitAttackPhase, FAttackContext&, Context);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCalculationPhase, FAttackContext&, Context, FHitInstance&, HitContext);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageApplicationPhase, FAttackContext&, Context, FHitInstance&,
                                               HitContext, FDamageResult&, Damage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectApplicationPhase, FAttackContext&, Context, FHitInstance&,
                                             HitContext);

UCLASS()
class KBS_API UTacCombatSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UTacAbilityExecutorService* GetAbilityExecutorService() const { return AbilityExecutorService; }
	UTacCombatStatisticsService* GetCombatStatisticsService() const { return CombatStatisticsService; }

	TArray<FCombatHitResult> ResolveAttack(AUnit* Attacker, TArray<AUnit*> Targets, class UWeapon* Weapon);
	TArray<FCombatHitResult> ResolveReactionAttack(AUnit* Attacker, TArray<AUnit*> Targets, class UWeapon* Weapon);
	bool ExecutePreAttackPhase(FAttackContext& Context);
	void ExecuteCalculationPhase(FAttackContext& Context, FHitInstance& Hit, FCombatHitResult& OutResult);
	void ExecuteDamageApplyPhase(FAttackContext& Context, FHitInstance& Hit, FCombatHitResult& ToApply);
	void ExecuteEffectApplicationPhase(FAttackContext& Context, FHitInstance& Hit, FCombatHitResult& Result);

	// Events
	UPROPERTY(BlueprintAssignable)
	FOnPreUnitAttackPhase OnPreUnitAttackPhase;
	UPROPERTY(BlueprintAssignable)
	FOnCalculationPhase OnCalculationPhase;
	UPROPERTY(BlueprintAssignable)
	FOnDamageApplicationPhase OnDamageApplicationPhase;
	UPROPERTY(BlueprintAssignable)
	FOnEffectApplicationPhase OnEffectApplicationPhase;

private:
	TArray<FCombatHitResult> ResolveAttackInternal(FAttackContext& Context);

	UPROPERTY()
	UTacAbilityExecutorService* AbilityExecutorService;
	UPROPERTY()
	UTacCombatStatisticsService* CombatStatisticsService;
};

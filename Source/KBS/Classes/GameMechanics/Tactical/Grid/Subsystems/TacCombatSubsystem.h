// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTypes/CombatTypes.h"
#include "TacCategoryLogger.h"
#include "TacCombatSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogKBSCombat, Log, All);

class UGridDataManager;
class UTacAbilityExecutorService;
class UTacCombatStatisticsService;
class AUnit;
class UWeapon;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreUnitAttackPhase, FCombatContext&, Context);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCalculationPhase, FCombatContext&, Context, FHitInstance&, HitContext);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageApplicationPhase, FCombatContext&, Context, FHitInstance&,
                                               HitContext, FDamageResult&, Damage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectApplicationPhase, FCombatContext&, Context, FHitInstance&,
                                             HitContext);

UCLASS()
class KBS_API UTacCombatSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UTacAbilityExecutorService* GetAbilityExecutorService() const { return AbilityExecutorService; }
	UTacCombatStatisticsService* GetCombatStatisticsService() const { return CombatStatisticsService; }

	TArray<FCombatHitResult> ResolveAttack(AUnit* Attacker, TArray<AUnit*> Targets, UWeapon*Weapon);
	TArray<FCombatHitResult> ResolveHealing(AUnit* Attacker, TArray<AUnit*> Targets, UWeapon*Weapon);
	TArray<FCombatHitResult> ResolveEffectApplication(AUnit* Attacker, TArray<AUnit*> Targets, UWeapon*Weapon);
	
	TArray<FCombatHitResult> ResolveReactionAttack(AUnit* Attacker, TArray<AUnit*> Targets, UWeapon*Weapon);
	
	bool ExecutePreResolutionPhase(FCombatContext& Context);
	void ExecuteCalculationPhase(FCombatContext& Context, FHitInstance& Hit, FCombatHitResult& OutResult);
	void ExecuteResultApplyPhase(FCombatContext& Context, FHitInstance& Hit, FCombatHitResult& ToApply);
	void ExecuteEffectApplicationPhase(FCombatContext& Context, FHitInstance& Hit, FCombatHitResult& Result);

	// Events
	UPROPERTY(BlueprintAssignable)
	FOnPreUnitAttackPhase OnPreResolutionPhase;
	UPROPERTY(BlueprintAssignable)
	FOnCalculationPhase OnCalculationPhase;
	UPROPERTY(BlueprintAssignable)
	FOnDamageApplicationPhase OnResultApplicationPhase;
	UPROPERTY(BlueprintAssignable)
	FOnEffectApplicationPhase OnEffectApplicationPhase;

private:
	TArray<FCombatHitResult> ResolveAttackInternal(FCombatContext& Context);
	void LogAttackStart(FCombatContext& Context);

	UPROPERTY()
	UTacAbilityExecutorService* AbilityExecutorService;
	UPROPERTY()
	UTacCombatStatisticsService* CombatStatisticsService;
	TUniquePtr<FTacCategoryLogger> CombatLogger;
};

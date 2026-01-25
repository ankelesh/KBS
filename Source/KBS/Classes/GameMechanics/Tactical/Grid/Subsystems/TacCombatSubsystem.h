// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Services/CombatMutationService.h"
#include "TacCombatSubsystem.generated.h"

class UCombatMutationService;
class UGridDataManager;
class UTacAbilityExecutorService;
class UTacCombatStatisticsService;
class AUnit;
UCLASS()
class KBS_API UTacCombatSubsystem : public UObject
{
	GENERATED_BODY()

	UTacCombatSubsystem();

public:

	void Initialize();
	UCombatMutationService* GetCombatMutationService() const { return CombatMutationService; }
	UTacAbilityExecutorService* GetAbilityExecutorService() const { return AbilityExecutorService; }
	UTacCombatStatisticsService* GetCombatStatisticsService() const { return CombatStatisticsService; }
	// Orchestration source of truth
	FMutationResult ProcessUnitHit(AUnit* Attacker, AUnit* Target);
	FMutationResults ProcessUnitHitMultiple(AUnit* Attacker, TArray<AUnit*> Targets);

private:
	UCombatMutationService* CombatMutationService;
	UTacAbilityExecutorService* AbilityExecutorService;
	UTacCombatStatisticsService* CombatStatisticsService;

};
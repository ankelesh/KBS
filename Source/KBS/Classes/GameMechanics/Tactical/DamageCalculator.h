// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTypes/CombatTypes.h"
#include "DamageCalculator.generated.h"

class AUnit;
class UWeapon;
class UBattleEffect;
enum class ETargetReach : uint8;

UCLASS()
class KBS_API UDamageCalculator : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Core calculation methods
	UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
	float CalculateHitChance(AUnit* Attacker, UWeapon* Weapon, AUnit* Target);

	UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
	FDamageResult CalculateDamage(AUnit* Attacker, UWeapon* Weapon, AUnit* Target);

	UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
	float CalculateEffectApplication(AUnit* Attacker, UBattleEffect* Effect, AUnit* Target);

	// Utility methods
	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	UWeapon* SelectWeapon(AUnit* Attacker, AUnit* Target, ETargetReach ExpectedReach);

	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	UWeapon* SelectMaxReachWeapon(AUnit* Unit);

	UFUNCTION(BlueprintCallable, Category = "Combat|Preview")
	FPreviewHitResult PreviewDamage(AUnit* Attacker, AUnit* Target, ETargetReach ExpectedReach);

	UFUNCTION(BlueprintCallable, Category = "Combat|Hit")
	FCombatHitResult ProcessHit(AUnit* Attacker, AUnit* Target, ETargetReach ExpectedReach);

	UFUNCTION(BlueprintCallable, Category = "Combat|Hit")
	TArray<FCombatHitResult> ProcessAttack(const FHitInstance& HitInstance);

	// Statistics management
	UFUNCTION(BlueprintCallable, Category = "Combat|Statistics")
	void ProcessStatistics(const FCombatHitResult& Result, bool bIsAttackerTeam);

	UFUNCTION(BlueprintCallable, Category = "Combat|Statistics")
	void ResetStatistics();

	UFUNCTION(BlueprintPure, Category = "Combat|Statistics")
	const FTeamCombatStats& GetAttackerStats() const { return AttackerStatistic; }

	UFUNCTION(BlueprintPure, Category = "Combat|Statistics")
	const FTeamCombatStats& GetDefenderStats() const { return DefenderStatistic; }

private:
	UPROPERTY()
	FTeamCombatStats AttackerStatistic;

	UPROPERTY()
	FTeamCombatStats DefenderStatistic;

	// Helper methods
	bool PerformAccuracyRoll(float HitChance);
	EDamageSource SelectBestDamageSource(const TMap<EDamageSource, int32>& DamageSources, AUnit* Target);
};

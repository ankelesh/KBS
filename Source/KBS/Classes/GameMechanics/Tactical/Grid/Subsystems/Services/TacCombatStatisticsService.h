#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/CombatTypes.h"
#include "TacCombatStatisticsService.generated.h"

/**
 * Tracks combat statistics for tactical battles.
 * Stateful service - maintains per-battle stats.
 */
UCLASS()
class KBS_API UTacCombatStatisticsService : public UObject
{
	GENERATED_BODY()

public:
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
}; 

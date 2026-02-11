#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacCombatStatisticsService.h"


void UTacCombatStatisticsService::ProcessStatistics(const FCombatHitResult& Result, bool bIsAttackerTeam)
{
	FTeamCombatStats& Stats = bIsAttackerTeam ? AttackerStatistic : DefenderStatistic;
	if (Result.bHit)
	{
		Stats.Hits++;
		Stats.TotalDamage += Result.DamageResult.Damage;
	}
	else
	{
		Stats.Misses++;
	}
	Stats.EffectsApplied += Result.EffectsApplied;
}

void UTacCombatStatisticsService::ResetStatistics()
{
	AttackerStatistic.Reset();
	DefenderStatistic.Reset();
}


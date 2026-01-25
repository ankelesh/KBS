// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/CombatMutationService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacCombatStatisticsService.h"
#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacAbilityExecutorService.h"

UTacCombatSubsystem::UTacCombatSubsystem()
{
}

FMutationResult UTacCombatSubsystem::ProcessUnitHit(AUnit* Attacker, AUnit* Target)
{
	return FMutationResult();
}
FMutationResults UTacCombatSubsystem::ProcessUnitHitMultiple(AUnit* Attacker, TArray<AUnit*> Targets)
{
	return FMutationResults();
}
void UTacCombatSubsystem::Initialize()
{
	CombatMutationService = NewObject<UCombatMutationService>(this);
	AbilityExecutorService = NewObject<UTacAbilityExecutorService>(this);
	CombatStatisticsService = NewObject<UTacCombatStatisticsService>(this);
}

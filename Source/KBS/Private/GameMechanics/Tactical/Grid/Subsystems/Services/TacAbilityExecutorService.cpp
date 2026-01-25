// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacAbilityExecutorService.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameplayTypes/GridCoordinates.h"


FAbilityValidation UTacAbilityExecutorService::ValidateAbility(UUnitAbilityInstance* Ability, AUnit* Source, FTacCoordinates TargetCell) const
{
	return FAbilityValidation();
}
void UTacAbilityExecutorService::Initialize()
{

}
FAbilityResult UTacAbilityExecutorService::ExecuteAbility(UUnitAbilityInstance* Ability, AUnit* Source, FTacCoordinates TargetCell)
{
	return FAbilityResult();
}

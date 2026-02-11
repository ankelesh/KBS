// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameplayTypes/GridCoordinates.h"
#include "TacAbilityExecutorService.generated.h"

class AUnit;
class UUnitAbilityInstance;

UCLASS()
class KBS_API UTacAbilityExecutorService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize();
	// always single cell since user can not click more than one cell at a time
	bool ValidateAbility(UUnitAbilityInstance* Ability, AUnit* Source, FTacCoordinates TargetCell) const;
	bool ExecuteAbility(UUnitAbilityInstance* Ability, AUnit* Source, FTacCoordinates TargetCell);
};
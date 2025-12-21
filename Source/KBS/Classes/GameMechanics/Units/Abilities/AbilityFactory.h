// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilityFactory.generated.h"

class UUnitAbilityDefinition;
class UUnitAbilityInstance;
class AUnit;

UCLASS()
class KBS_API UAbilityFactory : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Creates and initializes an ability instance from a definition
	 * @param Definition The ability definition data asset
	 * @param Owner The unit that will own this ability
	 * @return Initialized ability instance, or nullptr if creation failed
	 */
	static UUnitAbilityInstance* CreateAbilityFromDefinition(UUnitAbilityDefinition* Definition, AUnit* Owner);
};

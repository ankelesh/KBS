#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTypes/DamageTypes.h"
#include "AbilityTypesLibrary.generated.h"

/**
 * Static helper library for ability-related type conversions
 */
UCLASS()
class KBS_API UAbilityTypesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Convert ETargetReach enum to display string
	 */
	UFUNCTION(BlueprintPure, Category = "Ability|Types")
	static FString TargetReachToString(ETargetReach Reach);
};

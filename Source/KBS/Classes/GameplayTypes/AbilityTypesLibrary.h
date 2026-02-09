#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTypes/DamageTypes.h"
#include "AbilityTypesLibrary.generated.h"

enum class EHighlightType : uint8;

UCLASS()
class KBS_API UAbilityTypesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Ability|Types")
	static FString TargetReachToString(ETargetReach Reach);

	UFUNCTION(BlueprintPure, Category = "Ability|Types")
	static EHighlightType TargetReachToHighlightType(ETargetReach Reach);
};

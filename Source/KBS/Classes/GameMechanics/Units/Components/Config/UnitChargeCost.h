#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UnitChargeCost.generated.h"

// Price in UUnitChargeComponent resource (not to be confused with ability uses).
USTRUCT(BlueprintType)
struct KBS_API FUnitChargeCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charges")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charges", meta = (ClampMin = "1"))
	int32 Amount = 1;
};

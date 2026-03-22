#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "GameplayTagContainer.h"
#include "GameMechanics/Units/Components/Config/UnitComponentConfig.h"
#include "UnitChargeComponentConfig.generated.h"

USTRUCT(BlueprintType)
struct KBS_API FUnitChargeComponentEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charges")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charges")
	int32 CounterMax = 0;

	// -1 = start at CounterMax
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charges")
	int32 CounterStart = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charges")
	TSoftObjectPtr<UTexture2D> Icon;
};

USTRUCT(BlueprintType)
struct KBS_API FUnitChargeComponentConfig : public FUnitComponentConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charges")
	TArray<FUnitChargeComponentEntry> Entries;
};

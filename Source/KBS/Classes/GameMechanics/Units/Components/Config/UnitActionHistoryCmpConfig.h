#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameMechanics/Units/Components/Config/UnitComponentConfig.h"
#include "UnitActionHistoryCmpConfig.generated.h"

USTRUCT(BlueprintType)
struct KBS_API FUnitActionHistoryCmpConfig : public FUnitComponentConfig
{
	GENERATED_BODY()

	// Ordered sequence of tags to track; each step matches when the used ability has that tag.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "History")
	TArray<FGameplayTag> TargetSequence;
};

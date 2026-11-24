#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameMechanics/Units/Components/Config/UnitComponentConfig.h"
#include "UnitActionHistoryCmpConfig.generated.h"

USTRUCT(BlueprintType)
struct KBS_API FUnitActionHistoryCmpConfig : public FUnitComponentConfig
{
	GENERATED_BODY()

	// Ability tags that count as actions. The first one an ability carries is its action; abilities with none are ignored.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "History")
	TArray<FGameplayTag> TrackedActions;

	// Ordered actions to collect (each must be tracked). Out-of-order actions are ignored, never reset progress.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "History")
	TArray<FGameplayTag> TargetSequence;
};

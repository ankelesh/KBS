#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameMechanics/Units/Components/Config/UnitComponentConfig.h"
#include "GameMechanics/Units/Combat/CombatDescriptorDataAsset.h"
#include "UnitActionHistoryCmpConfig.generated.h"

USTRUCT(BlueprintType)
struct KBS_API FUnitActionHistoryCmpConfig : public FUnitComponentConfig
{
	GENERATED_BODY()

	// Ordered sequence of tags to track; each step matches when the used ability has that tag.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "History")
	TArray<FGameplayTag> TargetSequence;

	// Maps a gameplay tag found in the last action to a CombatDescriptor to react with.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Binding")
	TMap<FGameplayTag, TObjectPtr<UCombatDescriptorDataAsset>> TagToDescriptorMap;
};

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UnitAnimationSet.generated.h"

class UAnimMontage;

UCLASS(BlueprintType)
class KBS_API UUnitAnimationSet : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// Tag-to-montage map. Lookup falls back to parent tags (e.g. Attack.Slash -> Attack).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> Montages;
};

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CombatDescriptorData.h"
#include "CombatDescriptorDataAsset.generated.h"

UCLASS(BlueprintType)
class KBS_API UCombatDescriptorDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor")
	FCombatDescriptorData Data;
};

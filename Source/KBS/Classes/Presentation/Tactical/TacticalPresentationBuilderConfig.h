#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TacticalPresentationBuilderConfig.generated.h"

// Optional override asset for UTacticalPresentationBuilder. Absent config -> actions/actors use their own defaults.
UCLASS(BlueprintType)
class KBS_API UTacticalPresentationBuilderConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId("TacticalPresentationBuilderConfig", GetFName()); }
};

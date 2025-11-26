#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UnitStats.h"
#include "UnitDefinition.generated.h"

class UWeaponDataAsset;
class USkeletalMesh;
class UAnimBlueprintGeneratedClass;

UCLASS(BlueprintType)
class KBS_API UUnitDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<USkeletalMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSubclassOf<UAnimInstance> AnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UTexture2D> Portrait;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FUnitCoreStats BaseStatsTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FUnitProgressionData BaseProgressionTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<UWeaponDataAsset>> DefaultWeapons;
};

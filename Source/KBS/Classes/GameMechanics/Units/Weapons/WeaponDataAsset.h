#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Weapon.h"
#include "WeaponDataAsset.generated.h"

class UBattleEffect;

UCLASS(BlueprintType)
class KBS_API UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	FWeaponStats BaseStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Effects")
	TArray<TSubclassOf<UBattleEffect>> EffectClasses;
};

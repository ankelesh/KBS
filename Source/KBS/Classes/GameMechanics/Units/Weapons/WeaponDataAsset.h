#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Weapon.h"
#include "WeaponDataAsset.generated.h"

class UBattleEffect;
class UStaticMesh;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visual")
	TSoftObjectPtr<UStaticMesh> WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visual")
	FName AttachSocketName = TEXT("weapon_socket");
};

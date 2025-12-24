#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Weapon.h"
#include "WeaponDataAsset.generated.h"
class UBattleEffect;
class UBattleEffectDataAsset;
class UAnimMontage;
USTRUCT(BlueprintType)
struct FWeaponEffectConfig
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UBattleEffect> EffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UBattleEffectDataAsset> EffectConfig;
};
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
	TArray<FWeaponEffectConfig> Effects;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Animation")
	TObjectPtr<UAnimMontage> AttackMontage;
};

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CombatDescriptor.h"
#include "GameplayTypes/CombatDescriptorTypes.h"
#include "CombatDescriptorDataAsset.generated.h"
class UBattleEffect;
class UBattleEffectDataAsset;
class UAnimMontage;
USTRUCT(BlueprintType)
struct FDescriptorEffectConfig
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UBattleEffect> EffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UBattleEffectDataAsset> EffectConfig;
};
UCLASS(BlueprintType)
class KBS_API UCombatDescriptorDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor")
	FText Name;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor")
	FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor Stats")
	FCombatDescriptorStats BaseStats;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor Effects")
	TArray<FDescriptorEffectConfig> Effects;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor")
	ECombatDescriptorDesignation Designation = ECombatDescriptorDesignation::AllPurpose;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor")
	ECombatIntent Intent = ECombatIntent::Auto;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	bool bIsImmutable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	bool bGuaranteedHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor Animation")
	TObjectPtr<UAnimMontage> AttackMontage;
};

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GameplayTypes/CombatDescriptorTypes.h"
#include "WeaponDataAsset.generated.h"

class UCombatDescriptorDataAsset;

constexpr int32 NoWeaponDamageOverride = -1;

UCLASS(BlueprintType)
class KBS_API UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UCombatDescriptorDataAsset> Descriptor;

	// Empty = use Descriptor's name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText NameOverride;

	// NoWeaponDamageOverride = use Descriptor's BaseMagnitude
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 DamageOverride = NoWeaponDamageOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	ECombatDescriptorDesignation Designation = ECombatDescriptorDesignation::AllPurpose;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText Description;

	// Tag used to request animation from the unit's UUnitAnimationSet (e.g. Animation.Attack.Slash)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FGameplayTag AnimTag;
};

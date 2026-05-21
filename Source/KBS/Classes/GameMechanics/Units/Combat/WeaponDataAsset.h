#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTypes/CombatDescriptorTypes.h"
#include "GameplayTypes/DescriptorOverridePolicies.h"
#include "GameMechanics/Units/Combat/CombatDescriptorData.h"
#include "WeaponDataAsset.generated.h"

class UCombatDescriptorDataAsset;

// Sentinel conventions for InlineDescriptor merge (used by EDescriptorOverridePolicy):
//   BaseMagnitude      → -1          (skip if -1)
//   AccuracyMultiplier → -1          (skip if -1)
//   DamageSources      → empty set   (skip if empty)
//   TargetReach        → None        (skip if None; FAreaShape ignored when None)
//   Effects            → empty array (skip if empty)
//   FText fields       → empty       (skip if empty)
//   Booleans           → no sentinel, always copied when policy includes them

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FGameplayTag AnimTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	ECombatDescriptorDesignation Designation = ECombatDescriptorDesignation::AllPurpose;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor")
	TObjectPtr<UCombatDescriptorDataAsset> DescriptorAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor")
	FCombatDescriptorData InlineDescriptor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor")
	EDescriptorOverridePolicy OverridePolicy = EDescriptorOverridePolicy::AssetOnly;

	// Sentinel: -1 (ignored). When set, overrides BaseMagnitude from any source after merge.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor")
	int32 DamageOverride = -1;
};

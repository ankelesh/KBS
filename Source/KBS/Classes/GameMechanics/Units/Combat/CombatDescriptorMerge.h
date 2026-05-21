#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Combat/WeaponDataAsset.h"

// Resolves FWeaponData into a single use-ready FCombatDescriptorData per OverridePolicy.
// AssetOnly: checkf(DescriptorAsset != nullptr).
// InlineOnly or null asset: returns InlineDescriptor as-is.
// Merge/MergeWithBools/Append: field-by-field merge using sentinel conventions.
FCombatDescriptorData MergeDescriptors(const FWeaponData& Weapon);

#include "GameMechanics/Units/Combat/Weapon.h"
#include "GameMechanics/Units/Combat/WeaponDataAsset.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Units/Combat/CombatDescriptorMerge.h"

void UWeapon::Initialize(UObject* Outer, const FWeaponData& Data)
{
	Config = Data;
	Descriptor = NewObject<UCombatDescriptor>(Outer);
	Descriptor->Initialize(Outer, MergeDescriptors(Data));
}

FGameplayTag UWeapon::GetAnimTag() const
{
	return Config.AnimTag;
}

FText UWeapon::GetDisplayName() const
{
	return Config.Name;
}

const FText& UWeapon::GetDescription() const
{
	return Config.Description;
}

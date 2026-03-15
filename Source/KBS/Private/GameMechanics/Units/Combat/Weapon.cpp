#include "GameMechanics/Units/Combat/Weapon.h"
#include "GameMechanics/Units/Combat/WeaponDataAsset.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Units/Combat/CombatDescriptorDataAsset.h"

void UWeapon::Initialize(UObject* Outer, UWeaponDataAsset* Data)
{
	checkf(Data, TEXT("UWeapon::Initialize: null WeaponDataAsset"));
	checkf(Data->Descriptor, TEXT("UWeapon::Initialize: WeaponDataAsset has no Descriptor"));
	Config = Data;
	Descriptor = NewObject<UCombatDescriptor>(Outer);
	Descriptor->Initialize(Outer, Data->Descriptor, Data->DamageOverride);
}

FGameplayTag UWeapon::GetAnimTag() const
{
	return Config->AnimTag;
}

FText UWeapon::GetDisplayName() const
{
	return Config->NameOverride.IsEmpty() ? Config->Descriptor->Name : Config->NameOverride;
}

const FText& UWeapon::GetDescription() const
{
	return Config->Description;
}

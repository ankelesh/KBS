#include "GameMechanics/Units/Abilities/WeaponSwapAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"

FAbilityResult UWeaponSwapAbility::ApplyAbilityEffect(const FAbilityBattleContext& Context)
{
	if (!Owner)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom,
			FText::FromString("No owner unit"));
	}

	if (!SpellWeapon)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom,
			FText::FromString("No spell weapon configured"));
	}

	// Get the weapons array from owner unit
	TArray<TObjectPtr<UWeapon>>& UnitWeapons = const_cast<TArray<TObjectPtr<UWeapon>>&>(Owner->GetWeapons());

	// Destroy all existing weapons
	for (UWeapon* Weapon : UnitWeapons)
	{
		if (Weapon)
		{
			Weapon->DestroyComponent();
		}
	}
	UnitWeapons.Empty();

	// Create and add the spell weapon
	UWeapon* NewWeapon = NewObject<UWeapon>(Owner, SpellWeapon);
	if (NewWeapon)
	{
		NewWeapon->RegisterComponent();
		NewWeapon->InitializeFromDataAsset();
		UnitWeapons.Add(NewWeapon);
	}
	else
	{
		return CreateFailureResult(EAbilityFailureReason::Custom,
			FText::FromString("Failed to create spell weapon"));
	}

	// Switch current ability to auto-attack
	UAbilityInventoryComponent* Inventory = Owner->GetAbilityInventory();
	if (Inventory)
	{
		Inventory->SelectAttackAbility();
	}

	// Consume spell charge
	ConsumeCharge();

	UE_LOG(LogTemp, Log, TEXT("Spell '%s' activated - weapon swapped and auto-attack equipped"),
		*Config->AbilityName);

	return CreateSuccessResult();
}

ETargetReach UWeaponSwapAbility::GetTargeting() const
{
	// This ability doesn't need targeting - it just swaps weapons
	return ETargetReach::None;
}

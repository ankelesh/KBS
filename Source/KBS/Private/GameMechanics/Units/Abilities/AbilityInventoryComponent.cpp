// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Tactical/UnitAbilitySubsystem.h"

UAbilityInventoryComponent::UAbilityInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAbilityInventoryComponent::EquipAbility(UUnitAbilityInstance* Ability)
{
	if (!Ability)
	{
		return;
	}

	CurrentActiveAbility = Ability;
	OnAbilityEquipped.Broadcast(Ability);
}

void UAbilityInventoryComponent::AddPassiveAbility(UUnitAbilityInstance* Ability)
{
	if (!Ability || !Ability->IsPassive())
	{
		return;
	}

	PassiveAbilities.Add(Ability);

	UWorld* World = GetWorld();
	if (World)
	{
		UUnitAbilitySubsystem* AbilitySubsystem = World->GetSubsystem<UUnitAbilitySubsystem>();
		if (AbilitySubsystem)
		{
			Ability->Subscribe();
			AbilitySubsystem->RegisterAbility(Ability);
		}
	}
}

void UAbilityInventoryComponent::RemovePassiveAbility(UUnitAbilityInstance* Ability)
{
	if (!Ability)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		UUnitAbilitySubsystem* AbilitySubsystem = World->GetSubsystem<UUnitAbilitySubsystem>();
		if (AbilitySubsystem)
		{
			Ability->Unsubscribe();
			AbilitySubsystem->UnregisterAbility(Ability);
		}
	}

	PassiveAbilities.Remove(Ability);
}

void UAbilityInventoryComponent::RegisterPassives()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UUnitAbilitySubsystem* AbilitySubsystem = World->GetSubsystem<UUnitAbilitySubsystem>();
	if (!AbilitySubsystem)
	{
		return;
	}

	for (UUnitAbilityInstance* Ability : PassiveAbilities)
	{
		if (Ability)
		{
			Ability->Subscribe();
			AbilitySubsystem->RegisterAbility(Ability);
		}
	}
}

void UAbilityInventoryComponent::UnregisterPassives()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UUnitAbilitySubsystem* AbilitySubsystem = World->GetSubsystem<UUnitAbilitySubsystem>();
	if (!AbilitySubsystem)
	{
		return;
	}

	for (UUnitAbilityInstance* Ability : PassiveAbilities)
	{
		if (Ability)
		{
			Ability->Unsubscribe();
			AbilitySubsystem->UnregisterAbility(Ability);
		}
	}
}

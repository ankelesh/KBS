#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Tactical/UnitAbilitySubsystem.h"
UAbilityInventoryComponent::UAbilityInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
UUnitAbilityInstance* UAbilityInventoryComponent::GetCurrentActiveAbility() const
{
	return CurrentActiveAbility;
}
TArray<UUnitAbilityInstance*> UAbilityInventoryComponent::GetAvailableActiveAbilities() const
{
	TArray<UUnitAbilityInstance*> Result;
	for (const TObjectPtr<UUnitAbilityInstance>& Ability : AvailableActiveAbilities)
	{
		Result.Add(Ability);
	}
	return Result;
}
TArray<UUnitAbilityInstance*> UAbilityInventoryComponent::GetPassiveAbilities() const
{
	TArray<UUnitAbilityInstance*> Result;
	for (const TObjectPtr<UUnitAbilityInstance>& Ability : PassiveAbilities)
	{
		Result.Add(Ability);
	}
	return Result;
}
void UAbilityInventoryComponent::EquipAbility(UUnitAbilityInstance* Ability)
{
	if (!Ability)
	{
		return;
	}
	CurrentActiveAbility = Ability;
}
void UAbilityInventoryComponent::EquipDefaultAbility()
{
	if (AvailableActiveAbilities.Num() > 0)
	{
		CurrentActiveAbility = AvailableActiveAbilities[0];
		UE_LOG(LogTemp, Log, TEXT("AbilityInventory: Equipped default ability"));
	}
	else
	{
		CurrentActiveAbility = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("AbilityInventory: No available active abilities to equip"));
	}
}
void UAbilityInventoryComponent::AddActiveAbility(UUnitAbilityInstance* Ability)
{
	if (!Ability || Ability->IsPassive())
	{
		return;
	}
	AvailableActiveAbilities.Add(Ability);
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
TArray<FAbilityDisplayData> UAbilityInventoryComponent::GetActiveAbilitiesDisplayData() const
{
	TArray<FAbilityDisplayData> DisplayDataArray;
	for (const TObjectPtr<UUnitAbilityInstance>& Ability : AvailableActiveAbilities)
	{
		if (Ability)
		{
			DisplayDataArray.Add(Ability->GetAbilityDisplayData());
		}
	}
	return DisplayDataArray;
}
TArray<FAbilityDisplayData> UAbilityInventoryComponent::GetPassiveAbilitiesDisplayData() const
{
	TArray<FAbilityDisplayData> DisplayDataArray;
	for (const TObjectPtr<UUnitAbilityInstance>& Ability : PassiveAbilities)
	{
		if (Ability)
		{
			DisplayDataArray.Add(Ability->GetAbilityDisplayData());
		}
	}
	return DisplayDataArray;
}

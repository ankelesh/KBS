#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Tactical/UnitAbilitySubsystem.h"
#include "GameMechanics/Units/Abilities/UnitAutoAttackAbility.h"
#include "GameMechanics/Units/Abilities/UnitMovementAbility.h"
#include "GameMechanics/Units/Abilities/UnitWaitAbility.h"
#include "GameMechanics/Units/Abilities/UnitDefendAbility.h"
#include "GameMechanics/Units/Abilities/UnitFleeAbility.h"
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
	if (DefaultAttackAbility)
	{
		Result.Add(DefaultAttackAbility);
	}
	if (DefaultMoveAbility)
	{
		Result.Add(DefaultMoveAbility);
	}
	if (DefaultWaitAbility)
	{
		Result.Add(DefaultWaitAbility);
	}
	if (DefaultDefendAbility)
	{
		Result.Add(DefaultDefendAbility);
	}
	if (DefaultFleeAbility)
	{
		Result.Add(DefaultFleeAbility);
	}
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
	if (DefaultAttackAbility)
	{
		CurrentActiveAbility = DefaultAttackAbility;
		UE_LOG(LogTemp, Log, TEXT("AbilityInventory: Equipped Attack ability"));
	}
	else
	{
		CurrentActiveAbility = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("AbilityInventory: No Attack ability available to equip"));
	}
}
void UAbilityInventoryComponent::AddActiveAbility(UUnitAbilityInstance* Ability)
{
	if (!Ability || Ability->IsPassive())
	{
		return;
	}
	if (Ability->IsA<UUnitAutoAttackAbility>())
	{
		DefaultAttackAbility = Ability;
	}
	else if (Ability->IsA<UUnitMovementAbility>())
	{
		DefaultMoveAbility = Ability;
	}
	else if (Ability->IsA<UUnitWaitAbility>())
	{
		DefaultWaitAbility = Ability;
	}
	else if (Ability->IsA<UUnitDefendAbility>())
	{
		DefaultDefendAbility = Ability;
	}
	else if (Ability->IsA<UUnitFleeAbility>())
	{
		DefaultFleeAbility = Ability;
	}
	else
	{
		AvailableActiveAbilities.Add(Ability);
	}
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
	if (DefaultAttackAbility)
	{
		DisplayDataArray.Add(DefaultAttackAbility->GetAbilityDisplayData());
	}
	if (DefaultMoveAbility)
	{
		DisplayDataArray.Add(DefaultMoveAbility->GetAbilityDisplayData());
	}
	if (DefaultWaitAbility)
	{
		DisplayDataArray.Add(DefaultWaitAbility->GetAbilityDisplayData());
	}
	if (DefaultDefendAbility)
	{
		DisplayDataArray.Add(DefaultDefendAbility->GetAbilityDisplayData());
	}
	if (DefaultFleeAbility)
	{
		DisplayDataArray.Add(DefaultFleeAbility->GetAbilityDisplayData());
	}
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
void UAbilityInventoryComponent::SetDefaultAbility(EDefaultAbilitySlot Slot, UUnitAbilityInstance* Ability)
{
	switch (Slot)
	{
		case EDefaultAbilitySlot::Attack:
			DefaultAttackAbility = Ability;
			break;
		case EDefaultAbilitySlot::Move:
			DefaultMoveAbility = Ability;
			break;
		case EDefaultAbilitySlot::Wait:
			DefaultWaitAbility = Ability;
			break;
		case EDefaultAbilitySlot::Defend:
			DefaultDefendAbility = Ability;
			break;
		case EDefaultAbilitySlot::Flee:
			DefaultFleeAbility = Ability;
			break;
	}
}
UUnitAbilityInstance* UAbilityInventoryComponent::GetDefaultAbility(EDefaultAbilitySlot Slot) const
{
	switch (Slot)
	{
		case EDefaultAbilitySlot::Attack:
			return DefaultAttackAbility;
		case EDefaultAbilitySlot::Move:
			return DefaultMoveAbility;
		case EDefaultAbilitySlot::Wait:
			return DefaultWaitAbility;
		case EDefaultAbilitySlot::Defend:
			return DefaultDefendAbility;
		case EDefaultAbilitySlot::Flee:
			return DefaultFleeAbility;
		default:
			return nullptr;
	}
}
TArray<UUnitAbilityInstance*> UAbilityInventoryComponent::GetAllDefaultAbilities() const
{
	TArray<UUnitAbilityInstance*> Result;
	if (DefaultAttackAbility)
	{
		Result.Add(DefaultAttackAbility);
	}
	if (DefaultMoveAbility)
	{
		Result.Add(DefaultMoveAbility);
	}
	if (DefaultWaitAbility)
	{
		Result.Add(DefaultWaitAbility);
	}
	if (DefaultDefendAbility)
	{
		Result.Add(DefaultDefendAbility);
	}
	if (DefaultFleeAbility)
	{
		Result.Add(DefaultFleeAbility);
	}
	return Result;
}
void UAbilityInventoryComponent::SelectAttackAbility()
{
	if (DefaultAttackAbility)
	{
		CurrentActiveAbility = DefaultAttackAbility;
		UE_LOG(LogTemp, Log, TEXT("AbilityInventory: Auto-selected Attack ability for turn start"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityInventory: No Attack ability to auto-select"));
	}
}

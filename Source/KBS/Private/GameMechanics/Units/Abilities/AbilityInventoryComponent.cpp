#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacAbilityEventSubsystem.h"
#include "GameMechanics/Units/Abilities/UnitAutoAttackAbility.h"
#include "GameMechanics/Units/Abilities/UnitMovementAbility.h"
#include "GameMechanics/Units/Abilities/UnitWaitAbility.h"
#include "GameMechanics/Units/Abilities/UnitDefendAbility.h"
#include "GameMechanics/Units/Abilities/UnitFleeAbility.h"
#include "GameMechanics/Units/Unit.h"
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
	if (DefaultAttackAbility && IsAbilityAvailable(DefaultAttackAbility))
	{
		Result.Add(DefaultAttackAbility);
	}
	if (DefaultMoveAbility && IsAbilityAvailable(DefaultMoveAbility))
	{
		Result.Add(DefaultMoveAbility);
	}
	if (DefaultWaitAbility && IsAbilityAvailable(DefaultWaitAbility))
	{
		Result.Add(DefaultWaitAbility);
	}
	if (DefaultDefendAbility && IsAbilityAvailable(DefaultDefendAbility))
	{
		Result.Add(DefaultDefendAbility);
	}
	if (DefaultFleeAbility && IsAbilityAvailable(DefaultFleeAbility))
	{
		Result.Add(DefaultFleeAbility);
	}
	for (const TObjectPtr<UUnitAbilityInstance>& Ability : AvailableActiveAbilities)
	{
		if (IsAbilityAvailable(Ability))
		{
			Result.Add(Ability);
		}
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

	// Check if this is a spellbook ability
	if (Ability->GetConfig() && Ability->GetConfig()->bIsSpellbookAbility)
	{
		SpellbookAbilities.Add(Ability);
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
		UTacAbilityEventSubsystem* AbilitySubsystem = World->GetSubsystem<UTacAbilityEventSubsystem>();
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
		UTacAbilityEventSubsystem* AbilitySubsystem = World->GetSubsystem<UTacAbilityEventSubsystem>();
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
	UTacAbilityEventSubsystem* AbilitySubsystem = World->GetSubsystem<UTacAbilityEventSubsystem>();
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
	UTacAbilityEventSubsystem* AbilitySubsystem = World->GetSubsystem<UTacAbilityEventSubsystem>();
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
	if (DefaultAttackAbility && IsAbilityAvailable(DefaultAttackAbility))
	{
		DisplayDataArray.Add(DefaultAttackAbility->GetAbilityDisplayData());
	}
	if (DefaultMoveAbility && IsAbilityAvailable(DefaultMoveAbility))
	{
		DisplayDataArray.Add(DefaultMoveAbility->GetAbilityDisplayData());
	}
	if (DefaultWaitAbility && IsAbilityAvailable(DefaultWaitAbility))
	{
		DisplayDataArray.Add(DefaultWaitAbility->GetAbilityDisplayData());
	}
	if (DefaultDefendAbility && IsAbilityAvailable(DefaultDefendAbility))
	{
		DisplayDataArray.Add(DefaultDefendAbility->GetAbilityDisplayData());
	}
	if (DefaultFleeAbility && IsAbilityAvailable(DefaultFleeAbility))
	{
		DisplayDataArray.Add(DefaultFleeAbility->GetAbilityDisplayData());
	}
	for (const TObjectPtr<UUnitAbilityInstance>& Ability : AvailableActiveAbilities)
	{
		if (Ability && IsAbilityAvailable(Ability))
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

void UAbilityInventoryComponent::EnsureValidAbility()
{
	if (!CurrentActiveAbility)
	{
		EquipDefaultAbility();
		checkf(CurrentActiveAbility, TEXT("AbilityInventory: No valid ability after EquipDefaultAbility()"));
		return;
	}

	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	if (!OwnerUnit)
	{
		EquipDefaultAbility();
		checkf(CurrentActiveAbility, TEXT("AbilityInventory: No valid ability after EquipDefaultAbility()"));
		return;
	}

	if (!CurrentActiveAbility->CanExecute())
	{
		UE_LOG(LogTemp, Log, TEXT("AbilityInventory: Current ability '%s' not available, falling back to default attack"),
			*CurrentActiveAbility->GetConfig()->AbilityName);
		EquipDefaultAbility();
		checkf(CurrentActiveAbility, TEXT("AbilityInventory: No valid ability after EquipDefaultAbility()"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("AbilityInventory: Re-equipped last ability '%s'"),
			*CurrentActiveAbility->GetConfig()->AbilityName);
	}
}

bool UAbilityInventoryComponent::HasAnyAbilityAvailable() const
{
	const FUnitStatusContainer* Status = GetOwnerStatus();
	if (!Status)
	{
		return false;
	}

	// Quick early-out: if TurnBlocked or Fleeing, no abilities available
	if (!Status->CanAct() || Status->IsFleeing())
	{
		return false;
	}

	// Check default abilities
	if (DefaultAttackAbility && IsAbilityAvailable(DefaultAttackAbility))
	{
		return true;
	}
	if (DefaultMoveAbility && IsAbilityAvailable(DefaultMoveAbility))
	{
		return true;
	}
	if (DefaultWaitAbility && IsAbilityAvailable(DefaultWaitAbility))
	{
		return true;
	}
	if (DefaultDefendAbility && IsAbilityAvailable(DefaultDefendAbility))
	{
		return true;
	}
	if (DefaultFleeAbility && IsAbilityAvailable(DefaultFleeAbility))
	{
		return true;
	}

	// Check other active abilities
	for (const TObjectPtr<UUnitAbilityInstance>& Ability : AvailableActiveAbilities)
	{
		if (IsAbilityAvailable(Ability))
		{
			return true;
		}
	}

	return false;
}

// Spellbook methods
TArray<UUnitAbilityInstance*> UAbilityInventoryComponent::GetSpellbookAbilities() const
{
	TArray<UUnitAbilityInstance*> Result;
	if (!IsSpellbookAvailable())
	{
		return Result;
	}
	for (const TObjectPtr<UUnitAbilityInstance>& Ability : SpellbookAbilities)
	{
		Result.Add(Ability);
	}
	return Result;
}

TArray<FAbilityDisplayData> UAbilityInventoryComponent::GetSpellbookDisplayData() const
{
	TArray<FAbilityDisplayData> DisplayDataArray;
	if (!IsSpellbookAvailable())
	{
		return DisplayDataArray;
	}
	for (const TObjectPtr<UUnitAbilityInstance>& Ability : SpellbookAbilities)
	{
		if (Ability)
		{
			DisplayDataArray.Add(Ability->GetAbilityDisplayData());
		}
	}
	return DisplayDataArray;
}

bool UAbilityInventoryComponent::IsSpellbookAvailable() const
{
	if (GetOwnerStatus()->CanUseSpellbook() && !SpellbookAbilities.IsEmpty())
	{
		for (auto Ability : SpellbookAbilities)
		{
			if (Ability->CanExecute())
				return true;
		}
	}
	return false;
}

void UAbilityInventoryComponent::AddSpellbookAbility(UUnitAbilityInstance* Ability)
{
	if (!Ability)
	{
		return;
	}
	SpellbookAbilities.Add(Ability);
}

void UAbilityInventoryComponent::ActivateSpellbookSpell(UUnitAbilityInstance* Spell)
{
	checkf(!Spell, TEXT("AbilityInventory: Attempted to activate null spell"));
	if (Spell->CanExecute())
		EquipAbility(Spell);
}

const FUnitStatusContainer* UAbilityInventoryComponent::GetOwnerStatus() const
{
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	if (!OwnerUnit)
	{
		return nullptr;
	}
	return &OwnerUnit->GetStats().Status;
}

bool UAbilityInventoryComponent::IsDefaultAbility(UUnitAbilityInstance* Ability) const
{
	if (!Ability)
	{
		return false;
	}
	return Ability == DefaultAttackAbility ||
		   Ability == DefaultMoveAbility ||
		   Ability == DefaultWaitAbility ||
		   Ability == DefaultDefendAbility ||
		   Ability == DefaultFleeAbility;
}

bool UAbilityInventoryComponent::IsAbilityAvailable(UUnitAbilityInstance* Ability) const
{
	if (!Ability)
	{
		return false;
	}

	const FUnitStatusContainer* Status = GetOwnerStatus();
	if (!Status)
	{
		return false;
	}

	// TurnBlocked or Fleeing blocks all abilities
	if (!Status->CanAct() || Status->IsFleeing())
	{
		return false;
	}

	// Focused: only current ability available
	if (Status->IsFocused())
	{
		if (!CurrentActiveAbility)
		{
			UE_LOG(LogTemp, Error, TEXT("AbilityInventory: Unit is Focused but no ability equipped"));
			return false;
		}
		return Ability == CurrentActiveAbility;
	}

	// Disoriented: only default abilities available
	if (!Status->CanUseNonBasicAbilities())
	{
		return IsDefaultAbility(Ability);
	}

	// Silenced: blocks spellbook abilities
	if (!Status->CanUseSpellbook())
	{
		bool bIsSpellbookAbility = SpellbookAbilities.Contains(Ability);
		if (bIsSpellbookAbility)
		{
			return false;
		}
	}

	return true;
}

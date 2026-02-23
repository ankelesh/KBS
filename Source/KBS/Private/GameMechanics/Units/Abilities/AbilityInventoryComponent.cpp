#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Abilities/AbilityFactory.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacAbilityEventSubsystem.h"
#include "GameMechanics/Units/Abilities/Defaults/UnitAutoAttackAbility.h"
#include "GameMechanics/Units/Abilities/Defaults/UnitMovementAbility.h"
#include "GameMechanics/Units/Abilities/Defaults/UnitWaitAbility.h"
#include "GameMechanics/Units/Abilities/Defaults/UnitDefendAbility.h"
#include "GameMechanics/Units/Abilities/Defaults/UnitFleeAbility.h"
#include "GameMechanics/Units/Unit.h"
UAbilityInventoryComponent::UAbilityInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAbilityInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	checkf(OwnerUnit, TEXT("AbilityInventoryComponent must be owned by AUnit"));
	OwnerUnit->OnUnitTurnStart.AddDynamic(this, &UAbilityInventoryComponent::OnOwnerTurnStart);
	OwnerUnit->OnUnitTurnEnd.AddDynamic(this, &UAbilityInventoryComponent::OnOwnerTurnEnd);
}
void UAbilityInventoryComponent::InitializeFromDefinition(const UUnitDefinition* Definition, AUnit* OwnerUnit)
{
	auto CreateAndRegister = [&](UUnitAbilityDefinition* AbilityDef, EDefaultAbilitySlot Slot)
	{
		if (!AbilityDef) return;
		UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(AbilityDef, OwnerUnit);
		if (!Ability) return;
		SetDefaultAbility(Slot, Ability);
		AddActiveAbility(Ability);
	};
	CreateAndRegister(Definition->DefaultAttackAbility, EDefaultAbilitySlot::Attack);
	CreateAndRegister(Definition->DefaultMoveAbility,   EDefaultAbilitySlot::Move);
	CreateAndRegister(Definition->DefaultWaitAbility,   EDefaultAbilitySlot::Wait);
	CreateAndRegister(Definition->DefaultDefendAbility, EDefaultAbilitySlot::Defend);
	CreateAndRegister(Definition->DefaultFleeAbility,   EDefaultAbilitySlot::Flee);

	for (UUnitAbilityDefinition* AbilityDef : Definition->AdditionalAbilities)
	{
		if (!AbilityDef) continue;
		UUnitAbilityInstance* NewAbility = UAbilityFactory::CreateAbilityFromDefinition(AbilityDef, OwnerUnit);
		if (!NewAbility) continue;
		if (NewAbility->IsPassive()) AddPassiveAbility(NewAbility);
		else AddActiveAbility(NewAbility);
	}
	for (UUnitAbilityDefinition* AbilityDef : Definition->SpellbookAbilities)
	{
		if (!AbilityDef) continue;
		UUnitAbilityInstance* NewAbility = UAbilityFactory::CreateAbilityFromDefinition(AbilityDef, OwnerUnit);
		if (!NewAbility) continue;
		AddSpellbookAbility(NewAbility);
	}
	EquipDefaultAbility();
	// Passives are already subscribed individually inside AddPassiveAbility().
	// RegisterPassives() is reserved for re-registration after an explicit Unregister cycle.
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
	check(Ability);
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
	check(Ability);
	checkf(!Ability->IsPassive(), TEXT("AbilityInventory: passive ability passed to AddActiveAbility"));

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

	Ability->Subscribe();
}
void UAbilityInventoryComponent::AddPassiveAbility(UUnitAbilityInstance* Ability)
{
	check(Ability);
	checkf(Ability->IsPassive(), TEXT("AbilityInventory: non-passive ability passed to AddPassiveAbility"));
	PassiveAbilities.Add(Ability);
	UWorld* World = GetWorld();
	checkf(World, TEXT("AbilityInventory: no world when adding passive ability"));
	UTacAbilityEventSubsystem* AbilitySubsystem = World->GetSubsystem<UTacAbilityEventSubsystem>();
	checkf(AbilitySubsystem, TEXT("AbilityInventory: TacAbilityEventSubsystem not found"));
	Ability->Subscribe();
	AbilitySubsystem->RegisterAbility(Ability);
}
void UAbilityInventoryComponent::RemovePassiveAbility(UUnitAbilityInstance* Ability)
{
	check(Ability);
	UWorld* World = GetWorld();
	checkf(World, TEXT("AbilityInventory: no world when removing passive ability"));
	UTacAbilityEventSubsystem* AbilitySubsystem = World->GetSubsystem<UTacAbilityEventSubsystem>();
	checkf(AbilitySubsystem, TEXT("AbilityInventory: TacAbilityEventSubsystem not found"));
	Ability->Unsubscribe();
	AbilitySubsystem->UnregisterAbility(Ability);
	PassiveAbilities.Remove(Ability);
}
void UAbilityInventoryComponent::RegisterPassives()
{
	UWorld* World = GetWorld();
	checkf(World, TEXT("AbilityInventory: no world in RegisterPassives"));
	UTacAbilityEventSubsystem* AbilitySubsystem = World->GetSubsystem<UTacAbilityEventSubsystem>();
	checkf(AbilitySubsystem, TEXT("AbilityInventory: TacAbilityEventSubsystem not found"));
	for (UUnitAbilityInstance* Ability : PassiveAbilities)
	{
		if (Ability)
		{
			Ability->Subscribe();
			AbilitySubsystem->RegisterAbility(Ability);
		}
	}
}
void UAbilityInventoryComponent::OnOwnerTurnStart(AUnit* Unit)
{
	RecheckContents();
	EnsureValidAbility();
}

void UAbilityInventoryComponent::OnOwnerTurnEnd(AUnit* Unit)
{
	auto Notify = [](UUnitAbilityInstance* Ability) { if (Ability) Ability->HandleTurnEnd(); };

	Notify(DefaultAttackAbility);
	Notify(DefaultMoveAbility);
	Notify(DefaultWaitAbility);
	Notify(DefaultDefendAbility);
	Notify(DefaultFleeAbility);

	for (UUnitAbilityInstance* Ability : AvailableActiveAbilities)
		Notify(Ability);
	for (UUnitAbilityInstance* Ability : SpellbookAbilities)
		Notify(Ability);
	for (UUnitAbilityInstance* Ability : PassiveAbilities)
		Notify(Ability);
}

void UAbilityInventoryComponent::UnregisterPassives()
{
	UWorld* World = GetWorld();
	checkf(World, TEXT("AbilityInventory: no world in UnregisterPassives"));
	UTacAbilityEventSubsystem* AbilitySubsystem = World->GetSubsystem<UTacAbilityEventSubsystem>();
	checkf(AbilitySubsystem, TEXT("AbilityInventory: TacAbilityEventSubsystem not found"));
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

	// Quick early-out: if TurnBlocked or Fleeing, no abilities available
	if (!Status->CanAct() || Status->IsFleeing())
	{
		return false;
	}

	// Check default abilities
	if (IsAbilityAvailable(DefaultAttackAbility))
	{
		return true;
	}
	if (IsAbilityAvailable(DefaultMoveAbility))
	{
		return true;
	}
	if (IsAbilityAvailable(DefaultWaitAbility))
	{
		return true;
	}
	if (IsAbilityAvailable(DefaultDefendAbility))
	{
		return true;
	}
	if (IsAbilityAvailable(DefaultFleeAbility))
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
	check(Ability);
	SpellbookAbilities.Add(Ability);
	Ability->Subscribe();
}

void UAbilityInventoryComponent::ActivateSpellbookSpell(UUnitAbilityInstance* Spell)
{
	checkf(Spell, TEXT("AbilityInventory: Attempted to activate null spell"));
	if (Spell->CanExecute())
		EquipAbility(Spell);
}

bool UAbilityInventoryComponent::IsFocusedOn(const UUnitAbilityInstance* Ability) const
{
	return CurrentActiveAbility == Ability;
}

void UAbilityInventoryComponent::RecheckContents()
{
	for (auto Ability : SpellbookAbilities)
		Ability->RefreshAvailability();
	for (auto Ability : AvailableActiveAbilities)
		Ability->RefreshAvailability();
	DefaultAttackAbility->RefreshAvailability();
	DefaultMoveAbility->RefreshAvailability();
	DefaultWaitAbility->RefreshAvailability();
	DefaultDefendAbility->RefreshAvailability();
	DefaultFleeAbility->RefreshAvailability();
}

const FUnitStatusContainer* UAbilityInventoryComponent::GetOwnerStatus() const
{
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	checkf(OwnerUnit, TEXT("AbilityInventory: owner is not AUnit"));
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
		return Ability == CurrentActiveAbility && Ability->CanExecute();
	}

	// Disoriented: only default abilities available
	if (!Status->CanUseNonBasicAbilities())
	{
		return IsDefaultAbility(Ability) && Ability->CanExecute();
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

	return Ability->CanExecute();
}

#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Abilities/AbilityFactory.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacAbilityEventSubsystem.h"
#include "GameMechanics/Units/Unit.h"
UAbilityInventoryComponent::UAbilityInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	DefaultAbilities.SetNumZeroed(static_cast<int32>(EDefaultAbilitySlot::Count));
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
	auto AssignDefault = [&](UUnitAbilityDefinition* AbilityDef, EDefaultAbilitySlot Slot)
	{
		checkf(AbilityDef, TEXT("AbilityInventory: unit definition has no ability for default slot %s"),
		       *StaticEnum<EDefaultAbilitySlot>()->GetNameStringByValue(static_cast<int64>(Slot)));
		UUnitAbility* Ability = UAbilityFactory::CreateAbilityFromDefinition(AbilityDef, OwnerUnit);
		checkf(Ability, TEXT("AbilityInventory: factory failed to create default ability %s"), *AbilityDef->GetName());
		AssignDefaultAbility(Slot, Ability);
	};
	AssignDefault(Definition->DefaultAttackAbility, EDefaultAbilitySlot::Attack);
	AssignDefault(Definition->DefaultMoveAbility,   EDefaultAbilitySlot::Move);
	AssignDefault(Definition->DefaultWaitAbility,   EDefaultAbilitySlot::Wait);
	AssignDefault(Definition->DefaultDefendAbility, EDefaultAbilitySlot::Defend);
	AssignDefault(Definition->DefaultFleeAbility,   EDefaultAbilitySlot::Flee);

	for (UUnitAbilityDefinition* AbilityDef : Definition->AdditionalAbilities)
	{
		if (!AbilityDef) continue;
		UUnitAbility* NewAbility = UAbilityFactory::CreateAbilityFromDefinition(AbilityDef, OwnerUnit);
		if (!NewAbility) continue;
		if (NewAbility->IsPassive()) AddPassiveAbility(NewAbility);
		else AddOptionalAbility(NewAbility);
	}
	for (UUnitAbilityDefinition* AbilityDef : Definition->SpellbookAbilities)
	{
		if (!AbilityDef) continue;
		UUnitAbility* NewAbility = UAbilityFactory::CreateAbilityFromDefinition(AbilityDef, OwnerUnit);
		if (!NewAbility) continue;
		AddSpellbookAbility(NewAbility);
	}
	EnsureValidAbility();
	// Passives are already subscribed individually inside AddPassiveAbility().
	// RegisterPassives() is reserved for re-registration after an explicit Unregister cycle.
}
UUnitAbility* UAbilityInventoryComponent::GetCurrentActiveAbility() const
{
	return CurrentActiveAbility;
}
TArray<UUnitAbility*> UAbilityInventoryComponent::GetAvailableActiveAbilities() const
{
	TArray<UUnitAbility*> Result;
	for (const TObjectPtr<UUnitAbility>& Ability : AvailableActiveAbilities)
	{
		if (IsAbilityAvailable(Ability))
		{
			Result.Add(Ability);
		}
	}
	return Result;
}
TArray<UUnitAbility*> UAbilityInventoryComponent::GetPassiveAbilities() const
{
	TArray<UUnitAbility*> Result;
	for (const TObjectPtr<UUnitAbility>& Ability : PassiveAbilities)
	{
		Result.Add(Ability);
	}
	return Result;
}
void UAbilityInventoryComponent::EquipAbility(UUnitAbility* Ability)
{
	check(Ability);
	if (CurrentActiveAbility) CurrentActiveAbility->ChangeSelection(false);
	CurrentActiveAbility = Ability;
	CurrentActiveAbility->ChangeSelection(true);
}
void UAbilityInventoryComponent::AddOptionalAbility(UUnitAbility* Ability)
{
	check(Ability);
	checkf(!Ability->IsPassive(), TEXT("AbilityInventory: passive ability passed to AddOptionalAbility"));
	AvailableActiveAbilities.Add(Ability);
	Ability->Subscribe();
}
void UAbilityInventoryComponent::AssignDefaultAbility(EDefaultAbilitySlot Slot, UUnitAbility* Ability)
{
	check(Ability);
	checkf(!Ability->IsPassive(), TEXT("AbilityInventory: passive ability passed to AssignDefaultAbility"));
	checkf(Slot != EDefaultAbilitySlot::Count, TEXT("AbilityInventory: invalid default slot"));
	TObjectPtr<UUnitAbility>& SlotAbility = DefaultAbilities[static_cast<int32>(Slot)];
	checkf(!SlotAbility, TEXT("AbilityInventory: default slot already assigned"));
	SlotAbility = Ability;
	Ability->Subscribe();
}
void UAbilityInventoryComponent::AddPassiveAbility(UUnitAbility* Ability)
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
void UAbilityInventoryComponent::RemovePassiveAbility(UUnitAbility* Ability)
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
	for (UUnitAbility* Ability : PassiveAbilities)
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
	auto Notify = [](UUnitAbility* Ability) { if (Ability) Ability->HandleTurnEnd(); };

	for (UUnitAbility* Ability : DefaultAbilities)
		Notify(Ability);
	for (UUnitAbility* Ability : AvailableActiveAbilities)
		Notify(Ability);
	for (UUnitAbility* Ability : SpellbookAbilities)
		Notify(Ability);
	for (UUnitAbility* Ability : PassiveAbilities)
		Notify(Ability);
	AbilityContext.ClearTurnData();
}

void UAbilityInventoryComponent::UnregisterPassives()
{
	UWorld* World = GetWorld();
	checkf(World, TEXT("AbilityInventory: no world in UnregisterPassives"));
	UTacAbilityEventSubsystem* AbilitySubsystem = World->GetSubsystem<UTacAbilityEventSubsystem>();
	checkf(AbilitySubsystem, TEXT("AbilityInventory: TacAbilityEventSubsystem not found"));
	for (UUnitAbility* Ability : PassiveAbilities)
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
	for (const TObjectPtr<UUnitAbility>& Ability : DefaultAbilities)
	{
		if (IsAbilityAvailable(Ability))
		{
			DisplayDataArray.Add(Ability->GetAbilityDisplayData());
		}
	}
	for (const TObjectPtr<UUnitAbility>& Ability : AvailableActiveAbilities)
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
	for (const TObjectPtr<UUnitAbility>& Ability : PassiveAbilities)
	{
		if (Ability)
		{
			DisplayDataArray.Add(Ability->GetAbilityDisplayData());
		}
	}
	return DisplayDataArray;
}
UUnitAbility* UAbilityInventoryComponent::GetDefaultAbility(EDefaultAbilitySlot Slot) const
{
	checkf(Slot != EDefaultAbilitySlot::Count, TEXT("AbilityInventory: invalid default slot"));
	return DefaultAbilities[static_cast<int32>(Slot)];
}

void UAbilityInventoryComponent::EnsureValidAbility()
{
	if (CurrentActiveAbility && IsAbilityAvailable(CurrentActiveAbility))
	{
		return;
	}

	EquipAbility(GetDefaultAbility(EDefaultAbilitySlot::Attack));
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
	for (const TObjectPtr<UUnitAbility>& Ability : DefaultAbilities)
	{
		if (IsAbilityAvailable(Ability))
		{
			return true;
		}
	}

	// Check other active abilities
	for (const TObjectPtr<UUnitAbility>& Ability : AvailableActiveAbilities)
	{
		if (IsAbilityAvailable(Ability))
		{
			return true;
		}
	}

	return false;
}

// Spellbook methods
TArray<UUnitAbility*> UAbilityInventoryComponent::GetSpellbookAbilities() const
{
	TArray<UUnitAbility*> Result;
	if (!IsSpellbookAvailable())
	{
		return Result;
	}
	for (const TObjectPtr<UUnitAbility>& Ability : SpellbookAbilities)
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
	for (const TObjectPtr<UUnitAbility>& Ability : SpellbookAbilities)
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

void UAbilityInventoryComponent::AddSpellbookAbility(UUnitAbility* Ability)
{
	check(Ability);
	SpellbookAbilities.Add(Ability);
	Ability->Subscribe();
}


void UAbilityInventoryComponent::RecheckContents()
{
	for (auto Ability : SpellbookAbilities)
		Ability->RefreshAvailability();
	for (auto Ability : AvailableActiveAbilities)
		Ability->RefreshAvailability();
	for (auto Ability : DefaultAbilities)
		Ability->RefreshAvailability();
}

FAbilityContext* UAbilityInventoryComponent::GetContext()
{
	return &AbilityContext;
}

void UAbilityInventoryComponent::ProcessTurnPolicy(EAbilityTurnReleasePolicy Policy)
{
	switch (Policy)
	{
	case EAbilityTurnReleasePolicy::Free:
		if (AbilityContext.TurnState == EAbilityTurnReleasePolicy::Locked)
			AbilityContext.Unlock();
		break;
	case EAbilityTurnReleasePolicy::Locked:
		if (AbilityContext.TurnState != EAbilityTurnReleasePolicy::Locked)
			AbilityContext.Lock();
		break;
	case EAbilityTurnReleasePolicy::Conditional:
	case EAbilityTurnReleasePolicy::Released:
		AbilityContext.TurnState = Policy;
	}
}

const FUnitStatusContainer* UAbilityInventoryComponent::GetOwnerStatus() const
{
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	checkf(OwnerUnit, TEXT("AbilityInventory: owner is not AUnit"));
	return &OwnerUnit->GetStats().Status;
}

bool UAbilityInventoryComponent::IsDefaultAbility(UUnitAbility* Ability) const
{
	if (!Ability)
	{
		return false;
	}
	return DefaultAbilities.Contains(Ability);
}

bool UAbilityInventoryComponent::IsAbilityAvailable(UUnitAbility* Ability) const
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

FString UAbilityInventoryComponent::GetAbilitiesDebugString() const
{
	FString Result;
	auto Append = [&](const UUnitAbility* Ability, const TCHAR* Slot)
	{
		if (Ability)
			Result += FString::Printf(TEXT("  [%s] %s\n"), Slot, *Ability->GetDebugString());
	};

	const UEnum* SlotEnum = StaticEnum<EDefaultAbilitySlot>();
	for (int32 Index = 0; Index < DefaultAbilities.Num(); ++Index)
		Append(DefaultAbilities[Index], *SlotEnum->GetNameStringByIndex(Index));
	for (const TObjectPtr<UUnitAbility>& A : AvailableActiveAbilities)
		Append(A, TEXT("Active"));
	for (const TObjectPtr<UUnitAbility>& A : PassiveAbilities)
		Append(A, TEXT("Passive"));
	for (const TObjectPtr<UUnitAbility>& A : SpellbookAbilities)
		Append(A, TEXT("Spellbook"));

	return Result;
}

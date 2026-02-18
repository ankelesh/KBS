#include "UI/Tactical/HUD/Panels/AbilityPanel.h"
#include "UI/Tactical/HUD/Panels/DefaultAbilitiesPanel.h"
#include "UI/Tactical/HUD/Slots/ActiveAbilitySlot.h"
#include "UI/Tactical/HUD/Slots/SpellbookSlot.h"
#include "UI/Tactical/HUD/TacticalHUD.h"
#include "UI/Tactical/HUD/Popups/UnitSpellbookPopup.h"
#include "UI/Tactical/Controller/TacticalGameController.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "Components/HorizontalBox.h"

void UAbilityPanel::NativeConstruct()
{
	Super::NativeConstruct();

	checkf(DefaultAbilitiesPanel, TEXT("DefaultAbilitiesPanel not bound in AbilityPanel"));
	checkf(SpellbookSlot, TEXT("SpellbookSlot not bound in AbilityPanel"));
	checkf(ActiveAbilitySlotsContainer, TEXT("ActiveAbilitySlotsContainer not bound in AbilityPanel"));
	checkf(ActiveAbilitySlotClass, TEXT("ActiveAbilitySlotClass not set in AbilityPanel"));

	// Get subsystems - check once on init
	UWorld* World = GetWorld();
	checkf(World, TEXT("World is null in AbilityPanel::NativeConstruct"));

	TurnSubsystem = World->GetSubsystem<UTacTurnSubsystem>();
	checkf(TurnSubsystem, TEXT("TacTurnSubsystem not found"));

	GridSubsystem = World->GetSubsystem<UTacGridSubsystem>();
	checkf(GridSubsystem, TEXT("TacGridSubsystem not found"));

	// Get owning HUD
	APlayerController* PC = GetOwningPlayer();
	checkf(PC, TEXT("PlayerController is null in AbilityPanel::NativeConstruct"));

	ATacticalGameController* TacticalController = Cast<ATacticalGameController>(PC);
	checkf(TacticalController, TEXT("TacticalGameController not found in AbilityPanel"));

	OwningHUD = TacticalController->GetTacticalHUD();
	checkf(OwningHUD, TEXT("OwningHUD not found in AbilityPanel"));

	// Bind to TurnSubsystem


	// Bind to DefaultAbilitiesPanel
	DefaultAbilitiesPanel->OnAbilitySelected.AddDynamic(this, &UAbilityPanel::OnDefaultAbilitySelected);

	// Bind to Spellbook popup via HUD
	UUnitSpellbookPopup* SpellbookPopup = OwningHUD->GetSpellbookPopup();
	checkf(SpellbookPopup, TEXT("SpellbookPopup not found in AbilityPanel"));
	SpellbookPopup->OnAbilitySelected.AddDynamic(this, &UAbilityPanel::OnSpellbookAbilitySelected);

	// Create active ability slots
	ActiveAbilitySlots.Reserve(MAX_ACTIVE_ABILITY_SLOTS);
	for (int32 i = 0; i < MAX_ACTIVE_ABILITY_SLOTS; ++i)
	{
		UActiveAbilitySlot* AASlot = CreateWidget<UActiveAbilitySlot>(this, ActiveAbilitySlotClass);
		checkf(AASlot, TEXT("Failed to create ActiveAbilitySlot %d"), i);

		ActiveAbilitySlotsContainer->AddChild(AASlot);
		AASlot->OnAbilitySelected.AddDynamic(this, &UAbilityPanel::OnActiveAbilitySlotSelected);
		ActiveAbilitySlots.Add(AASlot);
	}
	TurnSubsystem->OnTurnStart.AddDynamic(this, &UAbilityPanel::OnTurnStarted);
	if (AUnit* InitFrom = TurnSubsystem->GetCurrentUnit())
	{
		OnTurnStarted(InitFrom);
	}
}

void UAbilityPanel::NativeDestruct()
{
	// Unbind from TurnSubsystem
	TurnSubsystem->OnTurnStart.RemoveDynamic(this, &UAbilityPanel::OnTurnStarted);

	// Unbind from DefaultAbilitiesPanel
	DefaultAbilitiesPanel->OnAbilitySelected.RemoveDynamic(this, &UAbilityPanel::OnDefaultAbilitySelected);

	// Unbind from active ability slots
	for (UActiveAbilitySlot* AASlot : ActiveAbilitySlots)
	{
		AASlot->OnAbilitySelected.RemoveDynamic(this, &UAbilityPanel::OnActiveAbilitySlotSelected);
	}

	// Unbind from spellbook popup
	UUnitSpellbookPopup* SpellbookPopup = OwningHUD->GetSpellbookPopup();
	SpellbookPopup->OnAbilitySelected.RemoveDynamic(this, &UAbilityPanel::OnSpellbookAbilitySelected);

	Super::NativeDestruct();
}

void UAbilityPanel::Clear()
{
	// Clear default abilities panel
	DefaultAbilitiesPanel->Clear();

	// Clear active ability slots
	for (UActiveAbilitySlot* AASlot : ActiveAbilitySlots)
	{
		AASlot->Clean();
	}

	// Clear spellbook slot
	SpellbookSlot->Clear();

	// Reset state
	CurrentUnit = nullptr;
	CurrentSelectedAbility = nullptr;
}

void UAbilityPanel::OnTurnStarted(AUnit* Unit)
{
	if (!Unit)
	{
		Clear();
		return;
	}

	// Check if unit belongs to player's team
	if (IsPlayerUnit(Unit))
	{
		SetUnit(Unit);
	}
	else
	{
		Clear();
	}
}

void UAbilityPanel::SetUnit(AUnit* Unit)
{
	Clear();

	if (!Unit)
	{
		return;
	}

	CurrentUnit = Unit;

	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();
	checkf(Inventory, TEXT("Unit has no AbilityInventoryComponent"));

	// Populate default abilities panel
	DefaultAbilitiesPanel->SetUnit(Unit);

	// Populate active ability slots
	PopulateActiveAbilitySlots(Unit);

	// Populate spellbook slot
	SpellbookSlot->SetupUnit(Unit);
}

void UAbilityPanel::PopulateActiveAbilitySlots(AUnit* Unit)
{
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();
	TArray<UUnitAbilityInstance*> AvailableAbilities = Inventory->GetAvailableActiveAbilities();

	// Limit to MAX_ACTIVE_ABILITY_SLOTS
	if (AvailableAbilities.Num() > MAX_ACTIVE_ABILITY_SLOTS)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityPanel: Unit has %d available active abilities, but only %d slots available. Showing first %d."),
			AvailableAbilities.Num(), MAX_ACTIVE_ABILITY_SLOTS, MAX_ACTIVE_ABILITY_SLOTS);
	}

	// Set abilities to slots
	int32 SlotIndex = 0;
	for (int32 i = 0; i < FMath::Min(AvailableAbilities.Num(), MAX_ACTIVE_ABILITY_SLOTS); ++i)
	{
		if (AvailableAbilities[i])
		{
			ActiveAbilitySlots[SlotIndex]->SetAbility(AvailableAbilities[i]);
			++SlotIndex;
		}
	}

	// Clean remaining slots
	for (int32 i = SlotIndex; i < MAX_ACTIVE_ABILITY_SLOTS; ++i)
	{
		ActiveAbilitySlots[i]->Clean();
	}
}

void UAbilityPanel::SelectAbility(UUnitAbilityInstance* Ability)
{
	if (!Ability)
	{
		return;
	}

	// Update cached selection
	CurrentSelectedAbility = Ability;

	// Deselect all first
	DeselectAllSlots();

	// Select in default panel
	DefaultAbilitiesPanel->SelectAbility(Ability);

	// Select in active ability slots
	for (UActiveAbilitySlot* AASlot : ActiveAbilitySlots)
	{
		if (AASlot->HasAbility(Ability))
		{
			AASlot->Select();
		}
	}

	// Note: SpellbookSlot selection is handled by the SpellbookPopup itself
}

void UAbilityPanel::DeselectAllSlots()
{
	// Deselect all default ability slots (handled by DefaultAbilitiesPanel internally)

	// Deselect all active ability slots
	for (UActiveAbilitySlot* AASlot : ActiveAbilitySlots)
	{
		AASlot->Deselect();
	}
}

void UAbilityPanel::OnDefaultAbilitySelected(UUnitAbilityInstance* Ability)
{
	if (!Ability)
	{
		return;
	}

	// Update selection state
	SelectAbility(Ability);

	// Broadcast to TurnSubsystem
	OnAbilitySelected.Broadcast(Ability);
}

void UAbilityPanel::OnActiveAbilitySlotSelected(UUnitAbilityInstance* Ability)
{
	if (!Ability)
	{
		return;
	}

	// Update selection state
	SelectAbility(Ability);

	// Broadcast to TurnSubsystem
	OnAbilitySelected.Broadcast(Ability);
}

void UAbilityPanel::OnSpellbookAbilitySelected(UUnitAbilityInstance* Ability)
{
	if (!Ability)
	{
		return;
	}

	// Update selection state
	SelectAbility(Ability);

	// Broadcast to TurnSubsystem
	OnAbilitySelected.Broadcast(Ability);
}

bool UAbilityPanel::IsPlayerUnit(AUnit* Unit) const
{
	if (!Unit)
	{
		return false;
	}

	// Use GridMetadata directly - contract guarantees GetPlayerTeam returns valid data
	UBattleTeam* PlayerTeam = GridSubsystem->GetPlayerTeam();
	ETeamSide PlayerTeamSide = PlayerTeam->GetTeamSide();
	ETeamSide UnitTeamSide = Unit->GetTeamSide();

	return UnitTeamSide == PlayerTeamSide;
}

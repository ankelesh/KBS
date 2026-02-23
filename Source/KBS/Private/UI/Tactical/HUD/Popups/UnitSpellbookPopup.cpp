#include "UI/Tactical/HUD/Popups/UnitSpellbookPopup.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "UI/Tactical/HUD/Slots/ActiveAbilitySlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"

DEFINE_LOG_CATEGORY(LogKBSUI);

void UUnitSpellbookPopup::NativeConstruct()
{
	Super::NativeConstruct();

	// Pre-create widget pool
	InitializePools();

	// Bind close button
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UUnitSpellbookPopup::OnCloseButtonClicked);
	}

	// Make widget focusable for modal behavior
	SetIsFocusable(true);
}

void UUnitSpellbookPopup::NativeDestruct()
{
	Super::NativeDestruct();
	Clear();

	// Unbind close button
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UUnitSpellbookPopup::OnCloseButtonClicked);
	}
}

FReply UUnitSpellbookPopup::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Right-click anywhere closes the popup
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (!bIsProcessingClick)
		{
			bIsProcessingClick = true;
			OnCloseRequired.Broadcast();
			bIsProcessingClick = false;
		}
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UUnitSpellbookPopup::InitializePools()
{
	// Pre-create ability slots (reserve 10 as specified)
	if (AbilitySlotClass && AbilityListContainer)
	{
		for (int32 i = 0; i < InitialAbilitySlots; ++i)
		{
			UActiveAbilitySlot* NewSlot = CreateWidget<UActiveAbilitySlot>(this, AbilitySlotClass);
			if (NewSlot)
			{
				NewSlot->SetVisibility(ESlateVisibility::Collapsed);
				AbilityListContainer->AddChild(NewSlot);
				AbilitySlotPool.Add(NewSlot);
			}
		}
	}
}

UActiveAbilitySlot* UUnitSpellbookPopup::GetOrCreateAbilitySlot()
{
	// Reuse existing slot from pool
	if (AbilitySlotsInUse < AbilitySlotPool.Num())
	{
		return AbilitySlotPool[AbilitySlotsInUse++];
	}

	// Pool exhausted, create new slot and add to pool
	if (AbilitySlotClass && AbilityListContainer)
	{
		UActiveAbilitySlot* NewSlot = CreateWidget<UActiveAbilitySlot>(this, AbilitySlotClass);
		if (NewSlot)
		{
			NewSlot->SetVisibility(ESlateVisibility::Collapsed);
			AbilityListContainer->AddChild(NewSlot);
			AbilitySlotPool.Add(NewSlot);
			AbilitySlotsInUse++;
			return NewSlot;
		}
	}

	return nullptr;
}

void UUnitSpellbookPopup::SetupFromUnit(AUnit* Unit)
{
	checkf(Unit, TEXT("UnitSpellbookPopup::SetupFromUnit - Unit is null"));

	// Clear existing data first
	Clear();

	// Re-read inventory and regenerate slots
	PopulateAbilities(Unit);

	// Reset multiclick protection
	bIsProcessingClick = false;
}

void UUnitSpellbookPopup::Clear()
{
	// Clear title
	if (TitleText)
	{
		TitleText->SetText(DefaultTitleText);
	}

	// Reset dynamic widgets (hides them, doesn't destroy)
	ResetAbilitySlots();

	// Reset multiclick protection
	bIsProcessingClick = false;
}

void UUnitSpellbookPopup::ResetAbilitySlots()
{
	// Clean and hide all currently used slots
	for (int32 i = 0; i < AbilitySlotsInUse && i < AbilitySlotPool.Num(); ++i)
	{
		if (AbilitySlotPool[i])
		{
			// Unbind from previous ability
			AbilitySlotPool[i]->OnAbilitySelected.RemoveDynamic(this, &UUnitSpellbookPopup::OnAbilitySlotClicked);
			AbilitySlotPool[i]->Clean();
		}
	}
	AbilitySlotsInUse = 0;
}

void UUnitSpellbookPopup::PopulateAbilities(AUnit* Unit)
{
	checkf(Unit, TEXT("UnitSpellbookPopup::PopulateAbilities - Unit is null"));

	UAbilityInventoryComponent* AbilityInventory = Unit->GetAbilityInventory();
	checkf(AbilityInventory, TEXT("UnitSpellbookPopup::PopulateAbilities - Unit has no AbilityInventory"));

	// Get spellbook abilities
	TArray<UUnitAbilityInstance*> SpellbookAbilities = AbilityInventory->GetSpellbookAbilities();

	for (UUnitAbilityInstance* Ability : SpellbookAbilities)
	{
		if (!Ability) continue;

		UActiveAbilitySlot* AbilitySlot = GetOrCreateAbilitySlot();
		if (AbilitySlot)
		{
			// Bind to ability and display it
			AbilitySlot->SetAbility(Ability);
			AbilitySlot->SetVisibility(ESlateVisibility::Visible);
			// Listen for ability selection
			AbilitySlot->OnAbilitySelected.AddDynamic(this, &UUnitSpellbookPopup::OnAbilitySlotClicked);
		}
	}
}

void UUnitSpellbookPopup::OnCloseButtonClicked()
{
	// Multiclick protection
	if (bIsProcessingClick) return;

	bIsProcessingClick = true;
	OnCloseRequired.Broadcast();
	bIsProcessingClick = false;
}

void UUnitSpellbookPopup::OnAbilitySlotClicked(UUnitAbilityInstance* Ability)
{
	// Multiclick protection
	if (bIsProcessingClick) return;

	bIsProcessingClick = true;

	// Emit ability selection
	OnAbilitySelected.Broadcast(Ability);

	// Close popup (quits on ability click)
	OnCloseRequired.Broadcast();

	bIsProcessingClick = false;
}

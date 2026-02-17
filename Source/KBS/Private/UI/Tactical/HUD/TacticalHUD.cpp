#include "UI/Tactical/HUD/TacticalHUD.h"
#include "UI/Tactical/HUD/Popups/UnitDetailsPopup.h"
#include "UI/Tactical/HUD/Popups/UnitSpellbookPopup.h"
#include "UI/Tactical/HUD/Panels/TurnQueuePanel.h"
#include "UI/Tactical/HUD/Panels/AbilityPanel.h"
#include "UI/Tactical/HUD/Panels/CurrentUnitPanel.h"
#include "UI/Tactical/HUD/Panels/HoveredUnitPanel.h"
#include "UI/Tactical/HUD/Panels/TeamPanel.h"
#include "UI/Tactical/HUD/Labels/TurnCounterLabel.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "Components/Overlay.h"

void UTacticalHUD::NativeConstruct()
{
	Super::NativeConstruct();

	TurnSubsystem = GetWorld()->GetSubsystem<UTacTurnSubsystem>();
	checkf(TurnSubsystem, TEXT("UTacticalHUD::NativeConstruct - TacTurnSubsystem not found"));

	// Add non-popup widgets to HUDLayout (they will have lower Z-order)
	if (TurnCounterLabelClass && HUDLayout)
	{
		TurnCounterLabel = CreateWidget<UTurnCounterLabel>(this, TurnCounterLabelClass);
		HUDLayout->AddChild(TurnCounterLabel);
	}

	if (TurnQueuePanelClass && HUDLayout)
	{
		TurnQueuePanel = CreateWidget<UTurnQueuePanel>(this, TurnQueuePanelClass);
		HUDLayout->AddChild(TurnQueuePanel);
	}

	if (TeamPanelClass && HUDLayout)
	{
		TeamPanel = CreateWidget<UTeamPanel>(this, TeamPanelClass);
		HUDLayout->AddChild(TeamPanel);
	}

	if (CurrentUnitPanelClass && HUDLayout)
	{
		CurrentUnitPanel = CreateWidget<UCurrentUnitPanel>(this, CurrentUnitPanelClass);
		HUDLayout->AddChild(CurrentUnitPanel);
	}

	if (HoveredUnitPanelClass && HUDLayout)
	{
		HoveredUnitPanel = CreateWidget<UHoveredUnitPanel>(this, HoveredUnitPanelClass);
		HUDLayout->AddChild(HoveredUnitPanel);
	}

	if (AbilityPanelClass && HUDLayout)
	{
		AbilityPanel = CreateWidget<UAbilityPanel>(this, AbilityPanelClass);
		HUDLayout->AddChild(AbilityPanel);
		AbilityPanel->OnAbilitySelected.AddDynamic(this, &UTacticalHUD::HandleAbilityPanelAbilitySelected);
	}

	// Create and cache popup instances (added last = highest Z-order)
	if (UnitDetailsPopupClass && HUDLayout)
	{
		UnitDetailsPopup = CreateWidget<UUnitDetailsPopup>(this, UnitDetailsPopupClass);
		if (UnitDetailsPopup)
		{
			HUDLayout->AddChild(UnitDetailsPopup);
			UnitDetailsPopup->OnCloseRequired.AddDynamic(this, &UTacticalHUD::OnUnitDetailsPopupCloseRequested);
			UnitDetailsPopup->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (SpellbookPopupClass && HUDLayout)
	{
		SpellbookPopup = CreateWidget<UUnitSpellbookPopup>(this, SpellbookPopupClass);
		if (SpellbookPopup)
		{
			HUDLayout->AddChild(SpellbookPopup);
			SpellbookPopup->OnCloseRequired.AddDynamic(this, &UTacticalHUD::OnSpellbookPopupCloseRequested);
			SpellbookPopup->OnAbilitySelected.AddDynamic(this, &UTacticalHUD::HandleSpellbookAbilitySelected);
			SpellbookPopup->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UTacticalHUD::ShowUnitDetailsPopup(AUnit* Unit)
{
	checkf(Unit, TEXT("UTacticalHUD::ShowUnitDetailsPopup - Unit is null"));
	UnitDetailsPopup->SetupFromUnit(Unit);
	ShowPopup(UnitDetailsPopup);
}

void UTacticalHUD::HideUnitDetailsPopup()
{
	UnitDetailsPopup->Clear();
	HidePopup(UnitDetailsPopup);
}

void UTacticalHUD::OnUnitDetailsPopupCloseRequested()
{
	HideUnitDetailsPopup();
}

void UTacticalHUD::ShowSpellbookPopup(AUnit* Unit)
{
	checkf(Unit, TEXT("UTacticalHUD::ShowSpellbookPopup - Unit is null"));
	SpellbookPopup->SetupFromUnit(Unit);
	ShowPopup(SpellbookPopup);
}

void UTacticalHUD::HideSpellbookPopup()
{
	SpellbookPopup->Clear();
	HidePopup(SpellbookPopup);
}

void UTacticalHUD::OnSpellbookPopupCloseRequested()
{
	HideSpellbookPopup();
}

void UTacticalHUD::HandleSpellbookAbilitySelected(UUnitAbilityInstance* Ability)
{
	AbilityPanel->SelectAbility(Ability);
	TurnSubsystem->AbilityClicked(Ability);
	OnSpellbookAbilitySelected.Broadcast(Ability);
}

void UTacticalHUD::HandleAbilityPanelAbilitySelected(UUnitAbilityInstance* Ability)
{
	TurnSubsystem->AbilityClicked(Ability);
}

void UTacticalHUD::SetHoveredUnit(AUnit* Unit)
{
	HoveredUnitPanel->SetHoveredUnit(Unit);
}

void UTacticalHUD::ClearHoveredUnit()
{
	HoveredUnitPanel->Clear();
}

void UTacticalHUD::ShowPopup(UUserWidget* PopupWidget)
{
	PopupWidget->SetVisibility(ESlateVisibility::Visible);
	PopupWidget->SetKeyboardFocus();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PopupWidget->TakeWidget());
	APlayerController* PC = GetOwningPlayer();
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
}

void UTacticalHUD::HidePopup(UUserWidget* PopupWidget)
{
	PopupWidget->SetVisibility(ESlateVisibility::Collapsed);

	APlayerController* PC = GetOwningPlayer();
	PC->SetInputMode(FInputModeGameOnly());
	PC->bShowMouseCursor = false;
}

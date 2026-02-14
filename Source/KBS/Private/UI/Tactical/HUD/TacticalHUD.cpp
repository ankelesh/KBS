#include "UI/Tactical/HUD/TacticalHUD.h"
#include "UI/Tactical/HUD/Popups/UnitDetailsPopup.h"
#include "GameMechanics/Units/Unit.h"
#include "Components/Overlay.h"

void UTacticalHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// Create and cache popup instance
	if (UnitDetailsPopupClass && PopupOverlay)
	{
		UnitDetailsPopup = CreateWidget<UUnitDetailsPopup>(this, UnitDetailsPopupClass);
		if (UnitDetailsPopup)
		{
			// Add to overlay with highest Z-order
			PopupOverlay->AddChild(UnitDetailsPopup);

			// Bind close event
			UnitDetailsPopup->OnCloseRequired.AddDynamic(this, &UTacticalHUD::OnUnitDetailsPopupCloseRequested);

			// Hide by default
			UnitDetailsPopup->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UTacticalHUD::ShowUnitDetailsPopup(AUnit* Unit)
{
	if (!UnitDetailsPopup)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTacticalHUD::ShowUnitDetailsPopup - UnitDetailsPopup is null"));
		return;
	}

	if (!Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTacticalHUD::ShowUnitDetailsPopup - Unit is null"));
		return;
	}

	// Setup popup with unit data
	UnitDetailsPopup->SetupFromUnit(Unit);

	// Show popup modally
	UnitDetailsPopup->SetVisibility(ESlateVisibility::Visible);

	// Bring to front (ensure highest Z-order)
	if (PopupOverlay)
	{
		PopupOverlay->RemoveChild(UnitDetailsPopup);
		PopupOverlay->AddChild(UnitDetailsPopup);
	}
}

void UTacticalHUD::HideUnitDetailsPopup()
{
	if (UnitDetailsPopup)
	{
		UnitDetailsPopup->SetVisibility(ESlateVisibility::Collapsed);
		UnitDetailsPopup->Clear();
	}
}

void UTacticalHUD::OnUnitDetailsPopupCloseRequested()
{
	HideUnitDetailsPopup();
}

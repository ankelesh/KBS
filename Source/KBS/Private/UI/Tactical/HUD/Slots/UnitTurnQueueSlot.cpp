#include "UI/Tactical/HUD/Slots/UnitTurnQueueSlot.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDisplayData.h"

void UUnitTurnQueueSlot::SetupSlot(const FUnitTurnQueueDisplay& DisplayData, int32 RolledInitiative, AUnit* Unit)
{
	TrackedUnit = Unit;

	// Set unit name
	if (UnitName)
	{
		UnitName->SetText(FText::FromString(DisplayData.UnitName));
		UnitName->SetVisibility(ESlateVisibility::Visible);
	}

	// Set initiative label
	if (InitiativeLabel)
	{
		FString InitiativeText = FString::Printf(TEXT("%d"), RolledInitiative);
		InitiativeLabel->SetText(FText::FromString(InitiativeText));
		InitiativeLabel->SetVisibility(ESlateVisibility::Visible);
	}

	// Set portrait
	if (Portrait && DisplayData.PortraitTexture)
	{
		Portrait->SetBrushFromTexture(DisplayData.PortraitTexture);
		Portrait->SetVisibility(ESlateVisibility::Visible);
	}

	// Make border visible
	if (SlotBorder)
	{
		SlotBorder->SetVisibility(ESlateVisibility::Visible);
	}

	// Call BP hook for custom styling
	BP_OnSetupSlot(DisplayData, RolledInitiative);
}

void UUnitTurnQueueSlot::Clear()
{
	// Clear text elements
	if (UnitName)
	{
		UnitName->SetText(FText::GetEmpty());
		UnitName->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (InitiativeLabel)
	{
		InitiativeLabel->SetText(FText::GetEmpty());
		InitiativeLabel->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Clear portrait
	if (Portrait)
	{
		Portrait->SetBrushFromTexture(nullptr);
		Portrait->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Hide border
	if (SlotBorder)
	{
		SlotBorder->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Call BP hook
	BP_OnClear();
}

void UUnitTurnQueueSlot::Clean()
{
	// Clear visual data
	Clear();

	// Unbind unit reference
	TrackedUnit = nullptr;

	// Call BP hook
	BP_OnClean();
}

FReply UUnitTurnQueueSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Handle right-click to emit details request
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton) && TrackedUnit)
	{
		OnDetailsRequested.Broadcast(TrackedUnit);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

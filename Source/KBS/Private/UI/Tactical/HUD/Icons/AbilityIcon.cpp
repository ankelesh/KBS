#include "UI/Tactical/HUD/Icons/AbilityIcon.h"
#include "Components/SizeBox.h"
#include "Components/Overlay.h"
#include "Blueprint/WidgetTree.h"
#include "Components/OverlaySlot.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameMechanics/Units/Abilities/AbilityDisplayData.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"

void UAbilityIcon::NativeConstruct()
{
	Super::NativeConstruct();

	// Create root SizeBox
	SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	WidgetTree->RootWidget = SizeBox;

	// Create SlotRoot overlay
	SlotRoot = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	SizeBox->AddChild(SlotRoot);

	// Create visual layers overlay
	UOverlay* VisualOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	SlotRoot->AddChildToOverlay(VisualOverlay);

	// Create SelectionBorder
	SelectionBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	VisualOverlay->AddChildToOverlay(SelectionBorder);

	// Create icon + charges overlay
	UOverlay* IconOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	VisualOverlay->AddChildToOverlay(IconOverlay);

	// Create AbilityIcon image
	AbilityIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	IconOverlay->AddChildToOverlay(AbilityIcon);

	// Create Charges text
	Charges = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Charges->SetText(FText::FromString(TEXT("0")));
	IconOverlay->AddChildToOverlay(Charges);

	// Create SlotButton
	SlotButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	SlotRoot->AddChildToOverlay(SlotButton);

	// Bind button click
	SlotButton->OnClicked.AddDynamic(this, &UAbilityIcon::OnSlotButtonClicked);
}

void UAbilityIcon::OnSlotButtonClicked()
{
	// TODO: Implement later
}

void UAbilityIcon::SetAbility(const FAbilityDisplayData& DisplayData)
{
	// Determine state
	if (DisplayData.bIsEmpty)
	{
		CurrentState = EIconState::IconEmpty;
	}
	else if (DisplayData.RemainingCharges == 0 || !DisplayData.bCanExecuteThisTurn)
	{
		CurrentState = EIconState::IconDisabled;
	}
	else
	{
		CurrentState = EIconState::IconSelected;
	}

	// Cache data
	CachedRemainingCharges = DisplayData.RemainingCharges;
	CachedMaxCharges = DisplayData.MaxCharges;

	// Set icon texture
	if (AbilityIcon && DisplayData.Icon)
	{
		AbilityIcon->SetBrushFromTexture(DisplayData.Icon);
	}

	// Apply visuals
	ApplyVisualState();
	UpdateChargesDisplay();
}

void UAbilityIcon::BindToAbility(UUnitAbilityInstance* Ability)
{
	if (Ability)
	{
		Ability->OnAbilityAvailabilityChange.AddDynamic(this, &UAbilityIcon::OnAbilityAvailabilityChanged);
	}
}

void UAbilityIcon::OnAbilityAvailabilityChanged(const UUnitAbilityInstance* Ability, bool bAvailable)
{
	if (!Ability) return;

	// Update state based on availability
	if (CachedRemainingCharges == 0 || !bAvailable)
	{
		CurrentState = EIconState::IconDisabled;
	}
	else
	{
		CurrentState = EIconState::IconSelected;
	}

	ApplyVisualState();
}

void UAbilityIcon::ApplyVisualState()
{
	if (!SizeBox) return;

	switch (CurrentState)
	{
		case EIconState::IconEmpty:
			SizeBox->SetVisibility(ESlateVisibility::Collapsed);
			break;

		case EIconState::IconDisabled:
			SizeBox->SetVisibility(ESlateVisibility::Visible);
			if (AbilityIcon)
			{
				AbilityIcon->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 0.5f));
			}
			break;

		case EIconState::IconSelected:
			SizeBox->SetVisibility(ESlateVisibility::Visible);
			if (AbilityIcon)
			{
				AbilityIcon->SetColorAndOpacity(FLinearColor::White);
			}
			break;
	}
}

void UAbilityIcon::UpdateChargesDisplay()
{
	if (!Charges) return;

	// Hide charges if max is 1 or less
	if (CachedMaxCharges <= 1)
	{
		Charges->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		Charges->SetVisibility(ESlateVisibility::Visible);
		FString ChargesText = FString::Printf(TEXT("%d/%d"), CachedRemainingCharges, CachedMaxCharges);
		Charges->SetText(FText::FromString(ChargesText));
	}
}

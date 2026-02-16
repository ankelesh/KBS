#include "UI/Tactical/HUD/Slots/ActiveAbilitySlot.h"
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

void UActiveAbilitySlot::NativeConstruct()
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
	SlotButton->OnClicked.AddDynamic(this, &UActiveAbilitySlot::OnSlotButtonClicked);
}

void UActiveAbilitySlot::NativeDestruct()
{
	if (BoundAbility)
	{
		BoundAbility->OnAbilityAvailabilityChange.RemoveDynamic(this, &UActiveAbilitySlot::OnAbilityAvailabilityChanged);
		BoundAbility->OnAbilityUsed.RemoveDynamic(this, &UActiveAbilitySlot::OnAbilityUsed);
		BoundAbility = nullptr;
	}
	Super::NativeDestruct();
}

void UActiveAbilitySlot::OnSlotButtonClicked()
{
	if (BoundAbility)
	{
		OnAbilitySelected.Broadcast(BoundAbility);
	}
}

void UActiveAbilitySlot::SetAbility(UUnitAbilityInstance* NewAbility)
{
	// Unbind from previous ability
	if (BoundAbility)
	{
		BoundAbility->OnAbilityAvailabilityChange.RemoveDynamic(this, &UActiveAbilitySlot::OnAbilityAvailabilityChanged);
		BoundAbility->OnAbilityUsed.RemoveDynamic(this, &UActiveAbilitySlot::OnAbilityUsed);
	}

	// Set new ability
	BoundAbility = NewAbility;

	// Transition to appropriate state
	if (!BoundAbility)
	{
		Hide();
	}
	else
	{
		// Bind to new ability
		BoundAbility->OnAbilityAvailabilityChange.AddDynamic(this, &UActiveAbilitySlot::OnAbilityAvailabilityChanged);
		BoundAbility->OnAbilityUsed.AddDynamic(this, &UActiveAbilitySlot::OnAbilityUsed);

		// Get display data
		FAbilityDisplayData DisplayData = BoundAbility->GetAbilityDisplayData();

		// Set icon texture
		if (AbilityIcon && DisplayData.Icon)
		{
			if (UMaterialInstanceDynamic* DynMaterial = AbilityIcon->GetDynamicMaterial())
			{
				// Set the texture parameter – must match the name in your material
				DynMaterial->SetTextureParameterValue(FName("InputTexture"), DisplayData.Icon);

				// Optionally set the grayscale amount (e.g., 0.0 for full color)
				DynMaterial->SetScalarParameterValue(FName("GrayScale"), 0.0f);
				AbilityIcon->SetVisibility(ESlateVisibility::Visible);
			}
		}

		// Determine initial state using display data - goes to Enabled, not Selected
		if (DisplayData.RemainingCharges == 0 || !DisplayData.bCanExecuteThisTurn)
		{
			CurrentState = EAbilitySlotState::Disabled;
			ApplyDisabledVisuals();
		}
		else
		{
			CurrentState = EAbilitySlotState::Enabled;
			ApplyEnabledVisuals();
		}
	}

	UpdateChargesDisplay();
}

void UActiveAbilitySlot::BindToAbility(UUnitAbilityInstance* Ability)
{
	// Unbind from previous ability
	if (BoundAbility)
	{
		BoundAbility->OnAbilityAvailabilityChange.RemoveDynamic(this, &UActiveAbilitySlot::OnAbilityAvailabilityChanged);
		BoundAbility->OnAbilityUsed.RemoveDynamic(this, &UActiveAbilitySlot::OnAbilityUsed);
	}

	BoundAbility = Ability;

	if (BoundAbility)
	{
		BoundAbility->OnAbilityAvailabilityChange.AddDynamic(this, &UActiveAbilitySlot::OnAbilityAvailabilityChanged);
		BoundAbility->OnAbilityUsed.AddDynamic(this, &UActiveAbilitySlot::OnAbilityUsed);
	}
}

void UActiveAbilitySlot::Clean()
{
	// Unbind from current ability
	if (BoundAbility)
	{
		BoundAbility->OnAbilityAvailabilityChange.RemoveDynamic(this, &UActiveAbilitySlot::OnAbilityAvailabilityChanged);
		BoundAbility->OnAbilityUsed.RemoveDynamic(this, &UActiveAbilitySlot::OnAbilityUsed);
		BoundAbility = nullptr;
	}

	// Clear visual data
	if (AbilityIcon)
	{
		AbilityIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Charges)
	{
		Charges->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Reset to hidden state
	Hide();
}

void UActiveAbilitySlot::OnAbilityAvailabilityChanged(const UUnitAbilityInstance* Ability, bool bAvailable)
{
	if (!Ability || Ability != BoundAbility) return;
	// Transition based on availability - always goes to Enabled, not Selected
	if (!bAvailable && (CurrentState == EAbilitySlotState::Enabled || CurrentState == EAbilitySlotState::Selected))
	{
		Disable();
	}
	else if (bAvailable && CurrentState == EAbilitySlotState::Disabled)
	{
		Enable();
	}
	UpdateChargesDisplay();
}

void UActiveAbilitySlot::OnAbilityUsed(int32 ChargesLeft, bool bAvailable)
{
	OnAbilityAvailabilityChanged(BoundAbility, bAvailable);
}

// State transition methods
void UActiveAbilitySlot::Select()
{
	if (CurrentState != EAbilitySlotState::Enabled) return;
	CurrentState = EAbilitySlotState::Selected;
	ApplySelectedVisuals();
}

void UActiveAbilitySlot::Deselect()
{
	if (CurrentState != EAbilitySlotState::Selected) return;
	CurrentState = EAbilitySlotState::Enabled;
	ApplyEnabledVisuals();
}

void UActiveAbilitySlot::Enable()
{
	if (CurrentState != EAbilitySlotState::Disabled) return;
	CurrentState = EAbilitySlotState::Enabled;
	ApplyEnabledVisuals();
}

void UActiveAbilitySlot::Disable()
{
	if (CurrentState == EAbilitySlotState::Hidden || CurrentState == EAbilitySlotState::Disabled) return;
	CurrentState = EAbilitySlotState::Disabled;
	ApplyDisabledVisuals();
}

void UActiveAbilitySlot::Hide()
{
	CurrentState = EAbilitySlotState::Hidden;
	ApplyHiddenVisuals();
}

bool UActiveAbilitySlot::HasAbility(UUnitAbilityInstance* Ability) const
{
	return (Ability == BoundAbility);
}

// Visual state helpers - C++ guarantees essential state, BP hooks add custom styling
void UActiveAbilitySlot::ApplySelectedVisuals()
{
	// Essential C++ logic (always runs)
	if (SizeBox) SizeBox->SetVisibility(ESlateVisibility::Visible);
	if (SlotButton) SlotButton->SetIsEnabled(true);

	// Call BP hook for custom styling
	BP_OnApplySelectedVisuals();
}

void UActiveAbilitySlot::ApplyEnabledVisuals()
{
	// Essential C++ logic (always runs)
	if (SizeBox) SizeBox->SetVisibility(ESlateVisibility::Visible);
	if (SlotButton) SlotButton->SetIsEnabled(true);

	// Call BP hook for custom styling
	BP_OnApplyEnabledVisuals();
}

void UActiveAbilitySlot::ApplyDisabledVisuals()
{
	// Essential C++ logic (always runs)
	if (SizeBox) SizeBox->SetVisibility(ESlateVisibility::Visible);
	if (SlotButton) SlotButton->SetIsEnabled(false);

	// Call BP hook for custom styling
	BP_OnApplyDisabledVisuals();
}

void UActiveAbilitySlot::ApplyHiddenVisuals()
{
	// Essential C++ logic (always runs)
	if (SizeBox) SizeBox->SetVisibility(ESlateVisibility::Collapsed);
	if (SlotButton) SlotButton->SetIsEnabled(false);

	// Call BP hook for custom styling
	BP_OnApplyHiddenVisuals();
}

void UActiveAbilitySlot::UpdateChargesDisplay()
{
	if (!Charges || !BoundAbility) return;

	// Show charges only if ability has explicit charges
	if (BoundAbility->HasExplicitCharges())
	{
		FAbilityDisplayData DisplayData = BoundAbility->GetAbilityDisplayData();
		Charges->SetVisibility(ESlateVisibility::Visible);
		FString ChargesText = FString::Printf(TEXT("%d/%d"), DisplayData.RemainingCharges, DisplayData.MaxCharges);
		Charges->SetText(FText::FromString(ChargesText));
	}
	else
	{
		Charges->SetVisibility(ESlateVisibility::Collapsed);
	}
}

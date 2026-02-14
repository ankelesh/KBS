#include "UI/Tactical/HUD/Snapshots/ActiveAbilitySlotSnapshot.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/AbilityDisplayData.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UActiveAbilitySlotSnapshot::SetupFromAbility(UUnitAbilityInstance* Ability)
{
	if (!Ability)
	{
		Clear();
		return;
	}

	FAbilityDisplayData DisplayData = Ability->GetAbilityDisplayData();

	// Set icon
	if (AbilityIcon && DisplayData.Icon)
	{
		AbilityIcon->SetBrushFromTexture(DisplayData.Icon);
		AbilityIcon->SetVisibility(ESlateVisibility::Visible);
	}

	// Set charges if ability has explicit charges
	if (ChargesText)
	{
		if (Ability->HasExplicitCharges())
		{
			FString ChargesStr = FString::Printf(TEXT("%d/%d"), DisplayData.RemainingCharges, DisplayData.MaxCharges);
			ChargesText->SetText(FText::FromString(ChargesStr));
			ChargesText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ChargesText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Set tooltip
	FText TooltipText = FText::Format(
		FText::FromString(TEXT("{0}\n{1}\n{2}")),
		FText::FromString(DisplayData.AbilityName),
		FText::FromString(DisplayData.Description),
		FText::FromString(DisplayData.TargetingInfo)
	);
	SetToolTipText(TooltipText);

	SetVisibility(ESlateVisibility::Visible);
}

void UActiveAbilitySlotSnapshot::Clear()
{
	if (AbilityIcon)
	{
		AbilityIcon->SetBrushFromTexture(nullptr);
		AbilityIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ChargesText)
	{
		ChargesText->SetText(FText::GetEmpty());
		ChargesText->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetToolTipText(FText::GetEmpty());
	SetVisibility(ESlateVisibility::Collapsed);
}

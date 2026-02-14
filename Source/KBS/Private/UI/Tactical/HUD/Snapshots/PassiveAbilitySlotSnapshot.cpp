#include "UI/Tactical/HUD/Snapshots/PassiveAbilitySlotSnapshot.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/AbilityDisplayData.h"
#include "Components/Image.h"

void UPassiveAbilitySlotSnapshot::SetupFromAbility(UUnitAbilityInstance* Ability)
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

	// Set tooltip
	FText TooltipText = FText::Format(
		FText::FromString(TEXT("{0}\n{1}")),
		FText::FromString(DisplayData.AbilityName),
		FText::FromString(DisplayData.Description)
	);
	SetToolTipText(TooltipText);

	SetVisibility(ESlateVisibility::Visible);
}

void UPassiveAbilitySlotSnapshot::Clear()
{
	if (AbilityIcon)
	{
		AbilityIcon->SetBrushFromTexture(nullptr);
		AbilityIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetToolTipText(FText::GetEmpty());
	SetVisibility(ESlateVisibility::Collapsed);
}

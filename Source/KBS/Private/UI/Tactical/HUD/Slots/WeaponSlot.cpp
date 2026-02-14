#include "UI/Tactical/HUD/Slots/WeaponSlot.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDisplayData.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "UI/Tactical/HUD/Snapshots//BattleEffectSlotSnapshot.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"

void UWeaponSlot::SetupFromWeapon(UWeapon* Weapon, AUnit* Owner)
{
	if (!Weapon)
	{
		Clear();
		return;
	}

	// Get display data
	FWeaponDisplayData DisplayData = ConvertWeapon(Weapon);

	// Set weapon name
	if (WeaponNameText)
	{
		WeaponNameText->SetText(FText::FromString(DisplayData.WeaponName));
	}

	// Set damage
	if (DamageText)
	{
		DamageText->SetText(FText::AsNumber(DisplayData.Damage));
	}

	// Set damage types
	if (DamageTypesText)
	{
		DamageTypesText->SetText(FText::FromString(DisplayData.DamageTypes));
	}

	// Set target type
	if (TargetTypeText)
	{
		TargetTypeText->SetText(FText::FromString(DisplayData.TargetType));
	}

	// Populate effects
	if (EffectIconList && EffectSlotClass)
	{
		EffectIconList->ClearChildren();
		CreatedEffectSlots.Empty();

		const TArray<UBattleEffect*>& Effects = Weapon->GetEffects();
		for (UBattleEffect* Effect : Effects)
		{
			if (!Effect) continue;

			UBattleEffectSlotSnapshot* EffectSlot = CreateWidget<UBattleEffectSlotSnapshot>(this, EffectSlotClass);
			if (EffectSlot)
			{
				EffectSlot->SetupFromEffect(Effect);
				EffectIconList->AddChild(EffectSlot);
				CreatedEffectSlots.Add(EffectSlot);
			}
		}
	}

	// Set tooltip with full weapon description
	if (Owner)
	{
		FText TooltipText = Weapon->GetEffectsTooltips(Owner);
		SetToolTipText(TooltipText);
	}

	SetVisibility(ESlateVisibility::Visible);
}

void UWeaponSlot::Clear()
{
	if (WeaponNameText)
	{
		WeaponNameText->SetText(FText::GetEmpty());
	}

	if (DamageText)
	{
		DamageText->SetText(FText::GetEmpty());
	}

	if (DamageTypesText)
	{
		DamageTypesText->SetText(FText::GetEmpty());
	}

	if (TargetTypeText)
	{
		TargetTypeText->SetText(FText::GetEmpty());
	}

	if (EffectIconList)
	{
		EffectIconList->ClearChildren();
	}

	CreatedEffectSlots.Empty();
	SetToolTipText(FText::GetEmpty());
	SetVisibility(ESlateVisibility::Collapsed);
}

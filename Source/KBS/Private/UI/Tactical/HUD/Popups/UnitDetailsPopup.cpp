#include "UI/Tactical/HUD/Popups/UnitDetailsPopup.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDisplayData.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "UI/Tactical/HUD/Snapshots/ActiveAbilitySlotSnapshot.h"
#include "UI/Tactical/HUD/Snapshots/PassiveAbilitySlotSnapshot.h"
#include "UI/Tactical/HUD/Snapshots/BattleEffectSlotSnapshot.h"
#include "UI/Tactical/HUD/Slots/WeaponSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"

void UUnitDetailsPopup::NativeConstruct()
{
	Super::NativeConstruct();

	// Pre-create widget pools
	InitializePools();

	// Make widget focusable for modal behavior
	SetIsFocusable(true);
	SetKeyboardFocus();
}

void UUnitDetailsPopup::NativeDestruct()
{
	Super::NativeDestruct();
	Clear();
}

FReply UUnitDetailsPopup::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Right-click anywhere closes the popup
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		OnCloseRequired.Broadcast();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UUnitDetailsPopup::InitializePools()
{
	// Pre-create weapon slots
	if (WeaponSlotClass && WeaponList)
	{
		for (int32 i = 0; i < InitialWeaponSlots; ++i)
		{
			UWeaponSlot* NewSlot = CreateWidget<UWeaponSlot>(this, WeaponSlotClass);
			if (NewSlot)
			{
				NewSlot->SetVisibility(ESlateVisibility::Collapsed);
				WeaponList->AddChild(NewSlot);
				WeaponSlotPool.Add(NewSlot);
			}
		}
	}

	// Pre-create active ability slots
	if (ActiveAbilitySlotClass && ActiveAbilityList)
	{
		for (int32 i = 0; i < InitialActiveAbilitySlots; ++i)
		{
			UActiveAbilitySlotSnapshot* NewSlot = CreateWidget<UActiveAbilitySlotSnapshot>(this, ActiveAbilitySlotClass);
			if (NewSlot)
			{
				NewSlot->SetVisibility(ESlateVisibility::Collapsed);
				ActiveAbilityList->AddChild(NewSlot);
				ActiveAbilitySlotPool.Add(NewSlot);
			}
		}
	}

	// Pre-create passive ability slots
	if (PassiveAbilitySlotClass && PassiveAbilityList)
	{
		for (int32 i = 0; i < InitialPassiveAbilitySlots; ++i)
		{
			UPassiveAbilitySlotSnapshot* NewSlot = CreateWidget<UPassiveAbilitySlotSnapshot>(this, PassiveAbilitySlotClass);
			if (NewSlot)
			{
				NewSlot->SetVisibility(ESlateVisibility::Collapsed);
				PassiveAbilityList->AddChild(NewSlot);
				PassiveAbilitySlotPool.Add(NewSlot);
			}
		}
	}

	// Pre-create effect slots
	if (BattleEffectSlotClass && EffectList)
	{
		for (int32 i = 0; i < InitialEffectSlots; ++i)
		{
			UBattleEffectSlotSnapshot* NewSlot = CreateWidget<UBattleEffectSlotSnapshot>(this, BattleEffectSlotClass);
			if (NewSlot)
			{
				NewSlot->SetVisibility(ESlateVisibility::Collapsed);
				EffectList->AddChild(NewSlot);
				EffectSlotPool.Add(NewSlot);
			}
		}
	}
}

UWeaponSlot* UUnitDetailsPopup::GetOrCreateWeaponSlot()
{
	// Reuse existing slot from pool
	if (WeaponSlotsInUse < WeaponSlotPool.Num())
	{
		return WeaponSlotPool[WeaponSlotsInUse++];
	}

	// Pool exhausted, create new slot and add to pool
	if (WeaponSlotClass && WeaponList)
	{
		UWeaponSlot* NewSlot = CreateWidget<UWeaponSlot>(this, WeaponSlotClass);
		if (NewSlot)
		{
			WeaponList->AddChild(NewSlot);
			WeaponSlotPool.Add(NewSlot);
			WeaponSlotsInUse++;
			return NewSlot;
		}
	}

	return nullptr;
}

UActiveAbilitySlotSnapshot* UUnitDetailsPopup::GetOrCreateActiveAbilitySlot()
{
	// Reuse existing slot from pool
	if (ActiveAbilitySlotsInUse < ActiveAbilitySlotPool.Num())
	{
		return ActiveAbilitySlotPool[ActiveAbilitySlotsInUse++];
	}

	// Pool exhausted, create new slot and add to pool
	if (ActiveAbilitySlotClass && ActiveAbilityList)
	{
		UActiveAbilitySlotSnapshot* NewSlot = CreateWidget<UActiveAbilitySlotSnapshot>(this, ActiveAbilitySlotClass);
		if (NewSlot)
		{
			ActiveAbilityList->AddChild(NewSlot);
			ActiveAbilitySlotPool.Add(NewSlot);
			ActiveAbilitySlotsInUse++;
			return NewSlot;
		}
	}

	return nullptr;
}

UPassiveAbilitySlotSnapshot* UUnitDetailsPopup::GetOrCreatePassiveAbilitySlot()
{
	// Reuse existing slot from pool
	if (PassiveAbilitySlotsInUse < PassiveAbilitySlotPool.Num())
	{
		return PassiveAbilitySlotPool[PassiveAbilitySlotsInUse++];
	}

	// Pool exhausted, create new slot and add to pool
	if (PassiveAbilitySlotClass && PassiveAbilityList)
	{
		UPassiveAbilitySlotSnapshot* NewSlot = CreateWidget<UPassiveAbilitySlotSnapshot>(this, PassiveAbilitySlotClass);
		if (NewSlot)
		{
			PassiveAbilityList->AddChild(NewSlot);
			PassiveAbilitySlotPool.Add(NewSlot);
			PassiveAbilitySlotsInUse++;
			return NewSlot;
		}
	}

	return nullptr;
}

UBattleEffectSlotSnapshot* UUnitDetailsPopup::GetOrCreateEffectSlot()
{
	// Reuse existing slot from pool
	if (EffectSlotsInUse < EffectSlotPool.Num())
	{
		return EffectSlotPool[EffectSlotsInUse++];
	}

	// Pool exhausted, create new slot and add to pool
	if (BattleEffectSlotClass && EffectList)
	{
		UBattleEffectSlotSnapshot* NewSlot = CreateWidget<UBattleEffectSlotSnapshot>(this, BattleEffectSlotClass);
		if (NewSlot)
		{
			EffectList->AddChild(NewSlot);
			EffectSlotPool.Add(NewSlot);
			EffectSlotsInUse++;
			return NewSlot;
		}
	}

	return nullptr;
}

void UUnitDetailsPopup::SetupFromUnit(AUnit* Unit)
{
	if (!Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("UnitDetailsPopup::SetupFromUnit - Unit is null"));
		return;
	}

	// Clear existing data first
	Clear();

	// Populate all sections
	PopulateStats(Unit);
	PopulateStatus(Unit);
	PopulateWeapons(Unit);
	PopulateAbilities(Unit);
	PopulateEffects(Unit);
}

void UUnitDetailsPopup::Clear()
{
	// Clear stats
	if (PortraitImage) PortraitImage->SetBrushFromTexture(nullptr);
	if (UnitNameText) UnitNameText->SetText(FText::GetEmpty());
	if (CurrentHealthText) CurrentHealthText->SetText(FText::GetEmpty());
	if (MaxHealthText) MaxHealthText->SetText(FText::GetEmpty());
	if (InitiativeText) InitiativeText->SetText(FText::GetEmpty());
	if (AccuracyText) AccuracyText->SetText(FText::GetEmpty());
	if (ArmourText) ArmourText->SetText(FText::GetEmpty());
	if (WardsText) WardsText->SetText(FText::GetEmpty());
	if (ImmunitiesText) ImmunitiesText->SetText(FText::GetEmpty());
	if (DamageReductionText) DamageReductionText->SetText(FText::GetEmpty());
	if (StatusText) StatusText->SetText(FText::GetEmpty());

	// Reset dynamic widgets (hides them, doesn't destroy)
	ResetWeaponSlots();
	ResetAbilitySlots();
	ResetEffectSlots();
}

void UUnitDetailsPopup::ResetWeaponSlots()
{
	// Clean and hide all currently used slots
	for (int32 i = 0; i < WeaponSlotsInUse && i < WeaponSlotPool.Num(); ++i)
	{
		if (WeaponSlotPool[i])
		{
			WeaponSlotPool[i]->Clear();
		}
	}
	WeaponSlotsInUse = 0;
}

void UUnitDetailsPopup::ResetAbilitySlots()
{
	// Clean and hide active ability slots
	for (int32 i = 0; i < ActiveAbilitySlotsInUse && i < ActiveAbilitySlotPool.Num(); ++i)
	{
		if (ActiveAbilitySlotPool[i])
		{
			ActiveAbilitySlotPool[i]->Clear();
		}
	}
	ActiveAbilitySlotsInUse = 0;

	// Clean and hide passive ability slots
	for (int32 i = 0; i < PassiveAbilitySlotsInUse && i < PassiveAbilitySlotPool.Num(); ++i)
	{
		if (PassiveAbilitySlotPool[i])
		{
			PassiveAbilitySlotPool[i]->Clear();
		}
	}
	PassiveAbilitySlotsInUse = 0;
}

void UUnitDetailsPopup::ResetEffectSlots()
{
	// Clean and hide effect slots
	for (int32 i = 0; i < EffectSlotsInUse && i < EffectSlotPool.Num(); ++i)
	{
		if (EffectSlotPool[i])
		{
			EffectSlotPool[i]->Clear();
		}
	}
	EffectSlotsInUse = 0;
}

void UUnitDetailsPopup::PopulateStats(AUnit* Unit)
{
	if (!Unit) return;

	FUnitDisplayData DisplayData = Unit->GetDisplayData();

	// Portrait
	if (PortraitImage && DisplayData.PortraitTexture)
	{
		PortraitImage->SetBrushFromTexture(DisplayData.PortraitTexture);
	}

	// Name
	if (UnitNameText)
	{
		UnitNameText->SetText(FText::FromString(DisplayData.UnitName));
	}

	// Health
	if (CurrentHealthText)
	{
		CurrentHealthText->SetText(FText::AsNumber(FMath::RoundToInt(DisplayData.CurrentHealth)));
	}
	if (MaxHealthText)
	{
		MaxHealthText->SetText(FText::AsNumber(FMath::RoundToInt(DisplayData.MaxHealth)));
	}

	// Initiative
	if (InitiativeText)
	{
		InitiativeText->SetText(FText::AsNumber(DisplayData.Initiative));
	}

	// Accuracy
	if (AccuracyText)
	{
		AccuracyText->SetText(FText::AsNumber(FMath::RoundToInt(DisplayData.Accuracy)));
	}

	// Defence stats
	if (ArmourText)
	{
		ArmourText->SetText(FText::FromString(DisplayData.Armour));
	}
	if (WardsText)
	{
		WardsText->SetText(FText::FromString(DisplayData.Wards));
	}
	if (ImmunitiesText)
	{
		ImmunitiesText->SetText(FText::FromString(DisplayData.Immunities));
	}
	if (DamageReductionText)
	{
		DamageReductionText->SetText(FText::AsNumber(DisplayData.DamageReduction));
	}
}

void UUnitDetailsPopup::PopulateStatus(AUnit* Unit)
{
	if (!Unit || !StatusText) return;

	FUnitDisplayData DisplayData = Unit->GetDisplayData();

	TArray<FString> StatusStrings;

	// Add defending status if active
	if (DisplayData.bIsDefending)
	{
		StatusStrings.Add(TEXT("Defending"));
	}

	// Add active effects as status indicators
	if (!DisplayData.ActiveEffectNames.IsEmpty())
	{
		StatusStrings.Add(DisplayData.ActiveEffectNames);
	}

	// Combine all status strings
	FString FinalStatus = StatusStrings.Num() > 0
		? FString::Join(StatusStrings, TEXT(", "))
		: TEXT("None");

	StatusText->SetText(FText::FromString(FinalStatus));
}

void UUnitDetailsPopup::PopulateWeapons(AUnit* Unit)
{
	if (!Unit) return;

	const TArray<TObjectPtr<FWeapon>>& Weapons = Unit->GetWeapons();

	for (UWeapon* Weapon : Weapons)
	{
		UWeaponSlot* WeaponSlot = GetOrCreateWeaponSlot();
		if (WeaponSlot)
		{
			WeaponSlot->SetupFromWeapon(Weapon, Unit);
		}
	}
}

void UUnitDetailsPopup::PopulateAbilities(AUnit* Unit)
{
	if (!Unit) return;

	UAbilityInventoryComponent* AbilityInventory = Unit->GetAbilityInventory();
	if (!AbilityInventory) return;

	// Populate active abilities
	{
		TArray<UUnitAbilityInstance*> ActiveAbilities = AbilityInventory->GetAvailableActiveAbilities();
		for (UUnitAbilityInstance* Ability : ActiveAbilities)
		{
			if (!Ability) continue;

			UActiveAbilitySlotSnapshot* AbilitySlot = GetOrCreateActiveAbilitySlot();
			if (AbilitySlot)
			{
				AbilitySlot->SetupFromAbility(Ability);
			}
		}
	}

	// Populate passive abilities
	{
		TArray<UUnitAbilityInstance*> PassiveAbilities = AbilityInventory->GetPassiveAbilities();
		for (UUnitAbilityInstance* Ability : PassiveAbilities)
		{
			if (!Ability) continue;

			UPassiveAbilitySlotSnapshot* AbilitySlot = GetOrCreatePassiveAbilitySlot();
			if (AbilitySlot)
			{
				AbilitySlot->SetupFromAbility(Ability);
			}
		}
	}
}

void UUnitDetailsPopup::PopulateEffects(AUnit* Unit)
{
	if (!Unit) return;

	UBattleEffectComponent* EffectComponent = Unit->EffectManager;
	if (!EffectComponent) return;

	const TArray<TObjectPtr<UBattleEffect>>& ActiveEffects = EffectComponent->GetActiveEffects();

	for (UBattleEffect* Effect : ActiveEffects)
	{
		if (!Effect) continue;

		UBattleEffectSlotSnapshot* EffectSlot = GetOrCreateEffectSlot();
		if (EffectSlot)
		{
			EffectSlot->SetupFromEffect(Effect);
		}
	}
}

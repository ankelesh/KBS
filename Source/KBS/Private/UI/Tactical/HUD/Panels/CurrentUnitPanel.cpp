#include "UI/Tactical/HUD/Panels/CurrentUnitPanel.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Units/Components/Config/UnitVisualDefinition.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "UI/Tactical/HUD/TacticalHUD.h"
#include "UI/Tactical/HUD/Slots/BattleEffectSlot.h"
#include "UI/Tactical/HUD/Slots/PassiveAbilitySlot.h"
#include "UI/Tactical/Controller/TacticalGameController.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"

void UCurrentUnitPanel::NativeConstruct()
{
	Super::NativeConstruct();

	// Get TurnSubsystem
	if (UWorld* World = GetWorld())
	{
		TurnSubsystem = World->GetSubsystem<UTacTurnSubsystem>();
		if (TurnSubsystem)
		{
			TurnSubsystem->OnTurnStart.AddDynamic(this, &UCurrentUnitPanel::OnTurnStarted);
			if (AUnit* Unit = TurnSubsystem->GetCurrentUnit())
			{
				RefreshPanel(Unit);
			}
		}
	}

	// Get owning HUD
	ATacticalGameController* TacticalController = Cast<ATacticalGameController>(GetOwningPlayer());
	checkf(TacticalController, TEXT("USpellbookSlot::OnSpellbookButtonClicked - TacticalGameController is null"));

	OwningHUD = TacticalController->GetTacticalHUD();
	checkf(OwningHUD, TEXT("USpellbookSlot::OnSpellbookButtonClicked - TacticalHUD is null"));

	// Bind portrait button
	if (PortraitButton)
	{
		PortraitButton->OnClicked.AddDynamic(this, &UCurrentUnitPanel::OnPortraitClicked);
	}

	// Reserve passive ability slots
	for (int32 i = 0; i < RESERVED_PASSIVE_SLOTS; ++i)
	{
		if (PassiveAbilitySlotClass && PassiveAbilitySlotContainer)
		{
			if (UPassiveAbilitySlot* PaSlot = CreateWidget<UPassiveAbilitySlot>(this, PassiveAbilitySlotClass))
			{
				PassiveAbilitySlotContainer->AddChild(PaSlot);
				PaSlot->Clear(); // Start collapsed
				PassiveAbilitySlots.Add(PaSlot);
			}
		}
	}
}

void UCurrentUnitPanel::NativeDestruct()
{
	Clear();

	if (TurnSubsystem)
	{
		TurnSubsystem->OnTurnStart.RemoveDynamic(this, &UCurrentUnitPanel::OnTurnStarted);
	}

	if (PortraitButton)
	{
		PortraitButton->OnClicked.RemoveDynamic(this, &UCurrentUnitPanel::OnPortraitClicked);
	}

	Super::NativeDestruct();
}

void UCurrentUnitPanel::Clear()
{
	UnbindFromCurrentUnit();

	// Clear portrait
	if (UnitPortraitImage)
	{
		UnitPortraitImage->SetBrushFromTexture(nullptr);
	}

	// Clear stats
	if (UnitNameText)
	{
		UnitNameText->SetText(FText::GetEmpty());
	}
	if (HealthText)
	{
		HealthText->SetText(FText::GetEmpty());
	}
	if (InitiativeText)
	{
		InitiativeText->SetText(FText::GetEmpty());
	}

	// Clear effect slots
	for (UBattleEffectSlot* PaSlot : EffectSlots)
	{
		if (PaSlot)
		{
			PaSlot->Clear();
		}
	}

	// Clear passive ability slots
	for (UPassiveAbilitySlot* PaSlot : PassiveAbilitySlots)
	{
		if (PaSlot)
		{
			PaSlot->Clear();
		}
	}

	CurrentUnit = nullptr;
}

void UCurrentUnitPanel::OnTurnStarted(AUnit* Unit)
{
	RefreshPanel(Unit);
}

void UCurrentUnitPanel::OnHealthChanged(AUnit* Unit, int32 NewHealth)
{
	if (Unit == CurrentUnit)
	{
		RefreshStats(Unit);
	}
}

void UCurrentUnitPanel::OnEffectsChanged()
{
	RefreshEffects(CurrentUnit);
}

void UCurrentUnitPanel::OnPortraitClicked()
{
	// Right-click on portrait invokes unit details popup
	if (OwningHUD && CurrentUnit)
	{
		OwningHUD->ShowUnitDetailsPopup(CurrentUnit);
	}
}

void UCurrentUnitPanel::RefreshPanel(AUnit* Unit)
{
	if (!Unit)
	{
		Clear();
		return;
	}

	UnbindFromCurrentUnit();
	CurrentUnit = Unit;

	// Bind to unit events
	CurrentUnit->OnHealthChanged.AddDynamic(this, &UCurrentUnitPanel::OnHealthChanged);
	CurrentUnit->EffectManager->OnActiveEffectsChanged.AddDynamic(this, &UCurrentUnitPanel::OnEffectsChanged);

	// Refresh all sections
	RefreshPortrait(Unit);
	RefreshStats(Unit);
	RefreshEffects(Unit);
	RefreshPassiveAbilities(Unit);

	// Notify Blueprint
	BP_OnUnitChanged(Unit);
}

void UCurrentUnitPanel::RefreshPortrait(AUnit* Unit)
{
	if (!Unit || !UnitPortraitImage)
	{
		return;
	}

	if (const UUnitVisualDefinition* VisualDef = Unit->GetActiveVisualDefinition(); VisualDef && VisualDef->Portrait)
	{
		UnitPortraitImage->SetBrushFromTexture(VisualDef->Portrait);
	}
	else
	{
		UnitPortraitImage->SetBrushFromTexture(nullptr);
	}
}

void UCurrentUnitPanel::RefreshStats(AUnit* Unit)
{
	if (!Unit)
	{
		return;
	}

	const FUnitCoreStats& Stats = Unit->GetStats();

	// Update unit name
	if (UnitNameText)
	{
		const UUnitDefinition* UnitDef = Unit->GetUnitDefinition();
		const FString Name = UnitDef ? UnitDef->UnitName : TEXT("Unknown");
		UnitNameText->SetText(FText::FromString(Name));
	}

	// Update HP (current/max)
	const int32 CurrentHP = Stats.Health.GetCurrent();
	const int32 MaxHP = Stats.Health.GetMaximum();
	if (HealthText)
	{
		const FString HPString = FString::Printf(TEXT("%d / %d"), CurrentHP, MaxHP);
		HealthText->SetText(FText::FromString(HPString));
	}

	// Notify Blueprint
	BP_OnHealthChanged(CurrentHP, MaxHP);

	// Update Initiative
	if (InitiativeText)
	{
		const int32 Initiative = Stats.Initiative.GetValue();
		InitiativeText->SetText(FText::AsNumber(Initiative));
	}
}

void UCurrentUnitPanel::RefreshEffects(AUnit* Unit)
{
	if (!Unit || !EffectSlotContainer)
	{
		return;
	}

	const TArray<TObjectPtr<UBattleEffect>>& ActiveEffects = Unit->EffectManager->GetActiveEffects();

	// Ensure we have enough slots (up to MAX_EFFECT_SLOTS)
	const int32 NumNeeded = FMath::Min(ActiveEffects.Num(), MAX_EFFECT_SLOTS);
	while (EffectSlots.Num() < NumNeeded)
	{
		UBattleEffectSlot* NewSlot = BattleEffectSlotClass ? CreateWidget<UBattleEffectSlot>(this, BattleEffectSlotClass) : nullptr;
		if (!NewSlot)
		{
			UE_LOG(LogTemp, Error, TEXT("CurrentUnitPanel: cannot create effect slot (BattleEffectSlotClass unset or creation failed)"));
			break;
		}
		EffectSlotContainer->AddChild(NewSlot);
		EffectSlots.Add(NewSlot);
	}

	// Setup slots with effects (slots auto-clear when effect is removed)
	for (int32 i = 0; i < EffectSlots.Num(); ++i)
	{
		if (i < ActiveEffects.Num())
		{
			EffectSlots[i]->SetupEffect(ActiveEffects[i]);
		}
		else
		{
			EffectSlots[i]->Clear();
		}
	}

	// Notify Blueprint
	BP_OnEffectsRefreshed(ActiveEffects.Num());
}

void UCurrentUnitPanel::RefreshPassiveAbilities(AUnit* Unit)
{
	if (!Unit || !PassiveAbilitySlotContainer)
	{
		return;
	}

	TArray<UUnitAbility*> PassiveAbilities = Unit->GetAbilityInventory()->GetPassiveAbilities();

	// Ensure we have enough slots (up to MAX_PASSIVE_SLOTS)
	const int32 NumNeeded = FMath::Min(PassiveAbilities.Num(), MAX_PASSIVE_SLOTS);
	while (PassiveAbilitySlots.Num() < NumNeeded)
	{
		UPassiveAbilitySlot* NewSlot = PassiveAbilitySlotClass ? CreateWidget<UPassiveAbilitySlot>(this, PassiveAbilitySlotClass) : nullptr;
		if (!NewSlot)
		{
			UE_LOG(LogTemp, Error, TEXT("CurrentUnitPanel: cannot create passive slot (PassiveAbilitySlotClass unset or creation failed)"));
			break;
		}
		PassiveAbilitySlotContainer->AddChild(NewSlot);
		PassiveAbilitySlots.Add(NewSlot);
	}

	// Setup slots with abilities
	for (int32 i = 0; i < PassiveAbilitySlots.Num(); ++i)
	{
		if (i < PassiveAbilities.Num())
		{
			PassiveAbilitySlots[i]->SetupFromAbility(PassiveAbilities[i], true);
		}
		else
		{
			PassiveAbilitySlots[i]->Clear();
		}
	}

	// Notify Blueprint
	BP_OnPassiveAbilitiesRefreshed(PassiveAbilities.Num());
}

void UCurrentUnitPanel::UnbindFromCurrentUnit()
{
	if (CurrentUnit)
	{
		CurrentUnit->OnHealthChanged.RemoveDynamic(this, &UCurrentUnitPanel::OnHealthChanged);
		CurrentUnit->EffectManager->OnActiveEffectsChanged.RemoveDynamic(this, &UCurrentUnitPanel::OnEffectsChanged);
	}
}

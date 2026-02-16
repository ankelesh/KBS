#include "UI/Tactical/HUD/Panels/HoveredUnitPanel.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "UI/Tactical/HUD/TacticalHUD.h"
#include "UI/Tactical/HUD/Snapshots/BattleEffectSlotSnapshot.h"
#include "UI/Tactical/Controller/TacticalGameController.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"

void UHoveredUnitPanel::NativeConstruct()
{
	Super::NativeConstruct();

	// Get owning HUD
	ATacticalGameController* TacticalController = Cast<ATacticalGameController>(GetOwningPlayer());
	checkf(TacticalController, TEXT("USpellbookSlot::OnSpellbookButtonClicked - TacticalGameController is null"));

	OwningHUD = TacticalController->GetTacticalHUD();
	checkf(OwningHUD, TEXT("USpellbookSlot::OnSpellbookButtonClicked - TacticalHUD is null"));

	// Bind portrait button for right-click
	if (PortraitButton)
	{
		PortraitButton->OnClicked.AddDynamic(this, &UHoveredUnitPanel::OnPortraitRightClicked);
	}
}

void UHoveredUnitPanel::NativeDestruct()
{
	Clear();

	if (PortraitButton)
	{
		PortraitButton->OnClicked.RemoveDynamic(this, &UHoveredUnitPanel::OnPortraitRightClicked);
	}

	Super::NativeDestruct();
}

void UHoveredUnitPanel::SetHoveredUnit(AUnit* Unit)
{
	// Reinit if unit changed
	if (Unit != HoveredUnit)
	{
		RefreshPanel(Unit);
	}
}

void UHoveredUnitPanel::Clear()
{
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

	// Clear effect snapshots
	for (UBattleEffectSlotSnapshot* Snapshot : EffectSnapshots)
	{
		if (Snapshot)
		{
			Snapshot->Clear();
		}
	}

	HoveredUnit = nullptr;
}

void UHoveredUnitPanel::OnPortraitRightClicked()
{
	// Right-click on portrait invokes unit details popup
	if (OwningHUD && HoveredUnit)
	{
		OwningHUD->ShowUnitDetailsPopup(HoveredUnit);
	}
}

void UHoveredUnitPanel::RefreshPanel(AUnit* Unit)
{
	if (!Unit)
	{
		Clear();
		return;
	}

	HoveredUnit = Unit;

	// Refresh all sections (snapshot only, no event binding)
	RefreshPortrait(Unit);
	RefreshStats(Unit);
	RefreshEffectSnapshots(Unit);

	// Notify Blueprint
	BP_OnHoveredUnitChanged(Unit);
}

void UHoveredUnitPanel::RefreshPortrait(AUnit* Unit)
{
	if (!Unit || !UnitPortraitImage)
	{
		return;
	}

	UUnitDefinition* UnitDef = Unit->GetUnitDefinition();
	if (UnitDef && UnitDef->Portrait)
	{
		UnitPortraitImage->SetBrushFromTexture(UnitDef->Portrait);
		// Portrait mirroring should be handled in Blueprint via RenderTransform
	}
	else
	{
		UnitPortraitImage->SetBrushFromTexture(nullptr);
	}
}

void UHoveredUnitPanel::RefreshStats(AUnit* Unit)
{
	if (!Unit)
	{
		return;
	}

	FUnitCoreStats& Stats = Unit->GetStats();

	// Update unit name
	if (UnitNameText)
	{
		UUnitDefinition* UnitDef = Unit->GetUnitDefinition();
		FString Name = UnitDef ? UnitDef->UnitName : TEXT("Unknown");
		UnitNameText->SetText(FText::FromString(Name));
	}

	// Update HP (current/max)
	int32 CurrentHP = Stats.Health.GetCurrent();
	int32 MaxHP = Stats.Health.GetMaximum();
	if (HealthText)
	{
		FString HPString = FString::Printf(TEXT("%d / %d"), CurrentHP, MaxHP);
		HealthText->SetText(FText::FromString(HPString));
	}

	// Update Initiative
	if (InitiativeText)
	{
		int32 Initiative = Stats.Initiative.GetValue();
		InitiativeText->SetText(FText::AsNumber(Initiative));
	}
}

void UHoveredUnitPanel::RefreshEffectSnapshots(AUnit* Unit)
{
	if (!Unit || !EffectSnapshotContainer)
	{
		return;
	}

	UBattleEffectComponent* EffectManager = Unit->EffectManager;
	if (!EffectManager)
	{
		return;
	}

	const TArray<TObjectPtr<UBattleEffect>>& ActiveEffects = EffectManager->GetActiveEffects();

	// Ensure we have enough snapshot slots (up to MAX_EFFECT_SNAPSHOTS)
	int32 NumNeeded = FMath::Min(ActiveEffects.Num(), MAX_EFFECT_SNAPSHOTS);
	while (EffectSnapshots.Num() < NumNeeded)
	{
		if (BattleEffectSlotSnapshotClass)
		{
			UBattleEffectSlotSnapshot* NewSnapshot = CreateWidget<UBattleEffectSlotSnapshot>(this, BattleEffectSlotSnapshotClass);
			if (NewSnapshot)
			{
				EffectSnapshotContainer->AddChild(NewSnapshot);
				EffectSnapshots.Add(NewSnapshot);
			}
		}
	}

	// Setup snapshot slots with effect data (read-only, no event binding)
	for (int32 i = 0; i < EffectSnapshots.Num(); ++i)
	{
		if (i < ActiveEffects.Num())
		{
			EffectSnapshots[i]->SetupFromEffect(ActiveEffects[i]);
		}
		else
		{
			EffectSnapshots[i]->Clear();
		}
	}
}

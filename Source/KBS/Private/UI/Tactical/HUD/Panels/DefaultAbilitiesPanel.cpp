#include "UI/Tactical/HUD/Panels/DefaultAbilitiesPanel.h"
#include "UI/Tactical/HUD/Slots/ActiveAbilitySlot.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetTree.h"

void UDefaultAbilitiesPanel::NativeConstruct()
{
	Super::NativeConstruct();

	CreateSlots();
}

void UDefaultAbilitiesPanel::NativeDestruct()
{
	if (AttackSlot)
	{
		AttackSlot->OnAbilitySelected.RemoveDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	}
	if (MoveSlot)
	{
		MoveSlot->OnAbilitySelected.RemoveDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	}
	if (WaitSlot)
	{
		WaitSlot->OnAbilitySelected.RemoveDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	}
	if (DefendSlot)
	{
		DefendSlot->OnAbilitySelected.RemoveDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	}
	if (FleeSlot)
	{
		FleeSlot->OnAbilitySelected.RemoveDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	}

	Super::NativeDestruct();
}

void UDefaultAbilitiesPanel::CreateSlots()
{
	checkf(ActiveAbilitySlotClass, TEXT("ActiveAbilitySlotClass not set in DefaultAbilitiesPanel"));
	checkf(SlotsContainer, TEXT("SlotsContainer not bound in DefaultAbilitiesPanel"));

	AttackSlot = CreateWidget<UActiveAbilitySlot>(this, ActiveAbilitySlotClass);
	MoveSlot = CreateWidget<UActiveAbilitySlot>(this, ActiveAbilitySlotClass);
	WaitSlot = CreateWidget<UActiveAbilitySlot>(this, ActiveAbilitySlotClass);
	DefendSlot = CreateWidget<UActiveAbilitySlot>(this, ActiveAbilitySlotClass);
	FleeSlot = CreateWidget<UActiveAbilitySlot>(this, ActiveAbilitySlotClass);

	int32 CreatedCount = 0;
	if (AttackSlot)
	{
		SlotsContainer->AddChildToHorizontalBox(AttackSlot);
		AttackSlot->OnAbilitySelected.AddDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
		++CreatedCount;
	}
	if (MoveSlot)
	{
		SlotsContainer->AddChildToHorizontalBox(MoveSlot);
		MoveSlot->OnAbilitySelected.AddDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
		++CreatedCount;
	}
	if (WaitSlot)
	{
		SlotsContainer->AddChildToHorizontalBox(WaitSlot);
		WaitSlot->OnAbilitySelected.AddDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
		++CreatedCount;
	}
	if (DefendSlot)
	{
		SlotsContainer->AddChildToHorizontalBox(DefendSlot);
		DefendSlot->OnAbilitySelected.AddDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
		++CreatedCount;
	}
	if (FleeSlot)
	{
		SlotsContainer->AddChildToHorizontalBox(FleeSlot);
		FleeSlot->OnAbilitySelected.AddDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
		++CreatedCount;
	}

	if (CreatedCount != REQUIRED_SLOTS)
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultAbilitiesPanel: Failed to create all %d slots. Only %d created."),
			REQUIRED_SLOTS, CreatedCount);
	}
}

void UDefaultAbilitiesPanel::Clear()
{
	if (AttackSlot) AttackSlot->Clean();
	if (MoveSlot) MoveSlot->Clean();
	if (WaitSlot) WaitSlot->Clean();
	if (DefendSlot) DefendSlot->Clean();
	if (FleeSlot) FleeSlot->Clean();
}

void UDefaultAbilitiesPanel::SetUnit(AUnit* Unit)
{
	Clear();

	if (!Unit)
	{
		return;
	}

	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();
	checkf(Inventory, TEXT("Unit has no AbilityInventoryComponent"));

	AttackSlot->SetAbility(Inventory->GetDefaultAbility(EDefaultAbilitySlot::Attack));
	MoveSlot->SetAbility(Inventory->GetDefaultAbility(EDefaultAbilitySlot::Move));
	WaitSlot->SetAbility(Inventory->GetDefaultAbility(EDefaultAbilitySlot::Wait));
	DefendSlot->SetAbility(Inventory->GetDefaultAbility(EDefaultAbilitySlot::Defend));
	FleeSlot->SetAbility(Inventory->GetDefaultAbility(EDefaultAbilitySlot::Flee));
}

void UDefaultAbilitiesPanel::SelectAbility(UUnitAbilityInstance* Ability)
{
	if (!Ability)
	{
		return;
	}

	// Deselect all first
	if (AttackSlot) AttackSlot->Deselect();
	if (MoveSlot) MoveSlot->Deselect();
	if (WaitSlot) WaitSlot->Deselect();
	if (DefendSlot) DefendSlot->Deselect();
	if (FleeSlot) FleeSlot->Deselect();

	// Select matching slot
	if (AttackSlot && AttackSlot->HasAbility(Ability))
	{
		AttackSlot->Select();
	}
	else if (MoveSlot && MoveSlot->HasAbility(Ability))
	{
		MoveSlot->Select();
	}
	else if (WaitSlot && WaitSlot->HasAbility(Ability))
	{
		WaitSlot->Select();
	}
	else if (DefendSlot && DefendSlot->HasAbility(Ability))
	{
		DefendSlot->Select();
	}
	else if (FleeSlot && FleeSlot->HasAbility(Ability))
	{
		FleeSlot->Select();
	}
}

void UDefaultAbilitiesPanel::OnChildAbilitySelected(UUnitAbilityInstance* Ability)
{
	// Transit selection event to parent
	OnAbilitySelected.Broadcast(Ability);
}

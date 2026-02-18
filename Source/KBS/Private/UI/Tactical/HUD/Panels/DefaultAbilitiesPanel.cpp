#include "UI/Tactical/HUD/Panels/DefaultAbilitiesPanel.h"
#include "UI/Tactical/HUD/Slots/ActiveAbilitySlot.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"

void UDefaultAbilitiesPanel::NativeConstruct()
{
	Super::NativeConstruct();

	AttackSlot->OnAbilitySelected.AddDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	MoveSlot->OnAbilitySelected.AddDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	WaitSlot->OnAbilitySelected.AddDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	DefendSlot->OnAbilitySelected.AddDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	FleeSlot->OnAbilitySelected.AddDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);

	Clear();
}

void UDefaultAbilitiesPanel::NativeDestruct()
{
	AttackSlot->OnAbilitySelected.RemoveDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	MoveSlot->OnAbilitySelected.RemoveDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	WaitSlot->OnAbilitySelected.RemoveDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	DefendSlot->OnAbilitySelected.RemoveDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);
	FleeSlot->OnAbilitySelected.RemoveDynamic(this, &UDefaultAbilitiesPanel::OnChildAbilitySelected);

	Super::NativeDestruct();
}

void UDefaultAbilitiesPanel::Clear()
{
	AttackSlot->Clean();
	MoveSlot->Clean();
	WaitSlot->Clean();
	DefendSlot->Clean();
	FleeSlot->Clean();
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
	AttackSlot->Deselect();
	MoveSlot->Deselect();
	WaitSlot->Deselect();
	DefendSlot->Deselect();
	FleeSlot->Deselect();

	// Select matching slot
	if (AttackSlot->HasAbility(Ability))
	{
		AttackSlot->Select();
	}
	else if (MoveSlot->HasAbility(Ability))
	{
		MoveSlot->Select();
	}
	else if (WaitSlot->HasAbility(Ability))
	{
		WaitSlot->Select();
	}
	else if (DefendSlot->HasAbility(Ability))
	{
		DefendSlot->Select();
	}
	else if (FleeSlot->HasAbility(Ability))
	{
		FleeSlot->Select();
	}
}

void UDefaultAbilitiesPanel::OnChildAbilitySelected(UUnitAbilityInstance* Ability)
{
	// Transit selection event to parent
	OnAbilitySelected.Broadcast(Ability);
}

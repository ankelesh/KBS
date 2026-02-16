#include "UI/Tactical/HUD/Slots/SpellbookSlot.h"
#include "UI/Tactical/HUD/TacticalHUD.h"
#include "UI/Tactical/HUD/Popups/UnitSpellbookPopup.h"
#include "UI/Tactical/HUD/Snapshots/ActiveAbilitySlotSnapshot.h"
#include "UI/Tactical/Controller/TacticalGameController.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "Components/Button.h"

void USpellbookSlot::NativeConstruct()
{
	Super::NativeConstruct();

	checkf(SpellbookButton, TEXT("USpellbookSlot::NativeConstruct - SpellbookButton is null"));
	SpellbookButton->OnClicked.AddDynamic(this, &USpellbookSlot::OnSpellbookButtonClicked);
}

void USpellbookSlot::NativeDestruct()
{
	Clear();
	Super::NativeDestruct();
}

void USpellbookSlot::SetupUnit(AUnit* InUnit)
{
	checkf(InUnit, TEXT("USpellbookSlot::SetupUnit - Unit is null"));

	UAbilityInventoryComponent* AbilityInventory = InUnit->GetAbilityInventory();
	checkf(AbilityInventory, TEXT("USpellbookSlot::SetupUnit - AbilityInventoryComponent is null"));

	// Clear ability slot state
	checkf(SpellbookSpellSlot, TEXT("USpellbookSlot::SetupUnit - SpellbookSpellSlot is null"));
	SpellbookSpellSlot->Clear();
	SpellbookSpellSlot->SetVisibility(ESlateVisibility::Collapsed);

	// Hide entire widget if no spellbook available
	const bool bHasSpellbook = AbilityInventory->IsSpellbookAvailable();
	SetVisibility(bHasSpellbook ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (!bHasSpellbook)
	{
		BoundUnit = nullptr;
		return;
	}

	BoundUnit = InUnit;

	// Bind to spellbook popup once (lazy initialization, auto-cleanup on widget destroy)
	if (!bDelegatesBound)
	{
		ATacticalGameController* TacticalController = Cast<ATacticalGameController>(GetOwningPlayer());
		checkf(TacticalController, TEXT("USpellbookSlot::SetupUnit - TacticalGameController is null"));

		UTacticalHUD* TacticalHUD = TacticalController->GetTacticalHUD();
		checkf(TacticalHUD, TEXT("USpellbookSlot::SetupUnit - TacticalHUD is null"));

		UUnitSpellbookPopup* SpellbookPopup = TacticalHUD->GetSpellbookPopup();
		checkf(SpellbookPopup, TEXT("USpellbookSlot::SetupUnit - SpellbookPopup is null"));

		SpellbookPopup->OnAbilitySelected.AddUniqueDynamic(this, &USpellbookSlot::OnSpellbookAbilitySelected);
		bDelegatesBound = true;
	}
}

void USpellbookSlot::Clear()
{
	checkf(SpellbookSpellSlot, TEXT("USpellbookSlot::Clear - SpellbookSpellSlot is null"));
	SpellbookSpellSlot->Clear();

	BoundUnit = nullptr;
	SetVisibility(ESlateVisibility::Collapsed);
}

void USpellbookSlot::OnSpellbookButtonClicked()
{
	checkf(BoundUnit.IsValid(), TEXT("USpellbookSlot::OnSpellbookButtonClicked - BoundUnit is invalid"));

	ATacticalGameController* TacticalController = Cast<ATacticalGameController>(GetOwningPlayer());
	checkf(TacticalController, TEXT("USpellbookSlot::OnSpellbookButtonClicked - TacticalGameController is null"));

	UTacticalHUD* TacticalHUD = TacticalController->GetTacticalHUD();
	checkf(TacticalHUD, TEXT("USpellbookSlot::OnSpellbookButtonClicked - TacticalHUD is null"));

	TacticalHUD->ShowSpellbookPopup(BoundUnit.Get());
}

void USpellbookSlot::OnSpellbookAbilitySelected(UUnitAbilityInstance* Ability)
{
	checkf(Ability, TEXT("USpellbookSlot::OnSpellbookAbilitySelected - Ability is null"));
	checkf(SpellbookSpellSlot, TEXT("USpellbookSlot::OnSpellbookAbilitySelected - SpellbookSpellSlot is null"));

	SpellbookSpellSlot->SetupFromAbility(Ability);
	SpellbookSpellSlot->SetVisibility(ESlateVisibility::Visible);
}

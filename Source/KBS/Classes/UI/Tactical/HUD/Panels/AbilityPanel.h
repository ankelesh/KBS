#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilityPanel.generated.h"

class AUnit;
class UDefaultAbilitiesPanel;
class UActiveAbilitySlot;
class USpellbookSlot;
class UUnitAbilityInstance;
class UTacTurnSubsystem;
class UTacGridSubsystem;
class UTacticalHUD;
class UUnitSpellbookPopup;
class UHorizontalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityPanelOnAbilitySelected, UUnitAbilityInstance*, Ability);

// Master panel wrapping AbilityInventory - manages DefaultAbilitiesPanel, active ability slots, and spellbook
// Listens to OnTurnStart, reinitializes for player units, clears for non-player units
// Synchronizes selection across all child slots
UCLASS(Blueprintable)
class KBS_API UAbilityPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	// Event fired when any ability is selected (from default panel, active slots, or spellbook)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FAbilityPanelOnAbilitySelected OnAbilitySelected;

	// Clean all child widgets and reset state
	UFUNCTION(BlueprintCallable, Category = "Ability Panel")
	void Clear();

	// Notify panel that ability selection changed externally (updates visual selection)
	UFUNCTION(BlueprintCallable, Category = "Ability Panel")
	void SelectAbility(UUnitAbilityInstance* Ability);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI Components - must be bound in BP
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UDefaultAbilitiesPanel> DefaultAbilitiesPanel;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<USpellbookSlot> SpellbookSlot;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UHorizontalBox> ActiveAbilitySlotsContainer;

	// Widget classes for dynamic creation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UActiveAbilitySlot> ActiveAbilitySlotClass;

private:
	UFUNCTION()
	void OnTurnStarted(AUnit* Unit);

	UFUNCTION()
	void OnDefaultAbilitySelected(UUnitAbilityInstance* Ability);

	UFUNCTION()
	void OnActiveAbilitySlotSelected(UUnitAbilityInstance* Ability);

	UFUNCTION()
	void OnSpellbookAbilitySelected(UUnitAbilityInstance* Ability);

	void SetUnit(AUnit* Unit);
	void PopulateActiveAbilitySlots(AUnit* Unit);
	void DeselectAllSlots();
	bool IsPlayerUnit(AUnit* Unit) const;

	// Cached subsystems and services
	UPROPERTY()
	TObjectPtr<UTacTurnSubsystem> TurnSubsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UTacGridSubsystem> GridSubsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UTacticalHUD> OwningHUD = nullptr;

	// Cached active ability slots (max 4, created on demand)
	UPROPERTY()
	TArray<TObjectPtr<UActiveAbilitySlot>> ActiveAbilitySlots;

	// Current unit being displayed
	UPROPERTY()
	TObjectPtr<AUnit> CurrentUnit = nullptr;

	// Currently selected ability for tracking
	UPROPERTY()
	TObjectPtr<UUnitAbilityInstance> CurrentSelectedAbility = nullptr;

	static constexpr int32 MAX_ACTIVE_ABILITY_SLOTS = 4;
};

#pragma once
#include "UI/Common/HUD/KbsHUD.h"
#include "TacticalHUD.generated.h"

class UUnitDetailsPopup;
class UUnitSpellbookPopup;
class UTurnQueuePanel;
class UTurnCounterLabel;
class UTeamPanel;
class UCurrentUnitPanel;
class UHoveredUnitPanel;
class UAbilityPanel;
class UOverlay;
class AUnit;
class UUnitAbilityInstance;
class UTacTurnSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpellbookAbilitySelected, UUnitAbilityInstance*, Ability);

UCLASS(Blueprintable)
class KBS_API UTacticalHUD : public UKbsHUD
{
	GENERATED_BODY()

public:
	// Event fired when an ability is selected from the spellbook (for linking to AbilityPanel)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSpellbookAbilitySelected OnSpellbookAbilitySelected;

	// Show unit details popup modally
	UFUNCTION(BlueprintCallable, Category = "Tactical HUD")
	void ShowUnitDetailsPopup(AUnit* Unit);

	// Hide unit details popup
	UFUNCTION(BlueprintCallable, Category = "Tactical HUD")
	void HideUnitDetailsPopup();

	// Show spellbook popup modally
	UFUNCTION(BlueprintCallable, Category = "Tactical HUD")
	void ShowSpellbookPopup(AUnit* Unit);

	// Hide spellbook popup
	UFUNCTION(BlueprintCallable, Category = "Tactical HUD")
	void HideSpellbookPopup();

	// Get spellbook popup instance (for direct event binding)
	UFUNCTION(BlueprintPure, Category = "Tactical HUD")
	UUnitSpellbookPopup* GetSpellbookPopup() const { return SpellbookPopup; }

	UFUNCTION(BlueprintCallable, Category = "Tactical HUD")
	void SetHoveredUnit(AUnit* Unit);

	UFUNCTION(BlueprintCallable, Category = "Tactical HUD")
	void ClearHoveredUnit();

protected:
	virtual void NativeConstruct() override;

	// Generic popup management with input locking
	void ShowPopup(UUserWidget* PopupWidget);
	void HidePopup(UUserWidget* PopupWidget);

	// Main HUD layout overlay (non-popup widgets at lower Z, popups on top)
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UOverlay* HUDLayout;

	// Widget class for unit details popup
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UUnitDetailsPopup> UnitDetailsPopupClass;

	// Widget class for spellbook popup
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UUnitSpellbookPopup> SpellbookPopupClass;

	// Widget class for turn queue panel
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UTurnQueuePanel> TurnQueuePanelClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UTurnCounterLabel> TurnCounterLabelClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UTeamPanel> TeamPanelClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UCurrentUnitPanel> CurrentUnitPanelClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UHoveredUnitPanel> HoveredUnitPanelClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UAbilityPanel> AbilityPanelClass;

private:
	UFUNCTION()
	void OnUnitDetailsPopupCloseRequested();

	UFUNCTION()
	void OnSpellbookPopupCloseRequested();

	UFUNCTION()
	void HandleSpellbookAbilitySelected(UUnitAbilityInstance* Ability);

	UFUNCTION()
	void HandleAbilityPanelAbilitySelected(UUnitAbilityInstance* Ability);

	// Cached popup instances (created on first use, hidden by default)
	UPROPERTY()
	TObjectPtr<UUnitDetailsPopup> UnitDetailsPopup;

	UPROPERTY()
	TObjectPtr<UUnitSpellbookPopup> SpellbookPopup;

	// Cached panel instances
	UPROPERTY()
	TObjectPtr<UTurnQueuePanel> TurnQueuePanel;

	UPROPERTY()
	TObjectPtr<UTurnCounterLabel> TurnCounterLabel;

	UPROPERTY()
	TObjectPtr<UTeamPanel> TeamPanel;

	UPROPERTY()
	TObjectPtr<UCurrentUnitPanel> CurrentUnitPanel;

	UPROPERTY()
	TObjectPtr<UHoveredUnitPanel> HoveredUnitPanel;

	UPROPERTY()
	TObjectPtr<UAbilityPanel> AbilityPanel;

	UPROPERTY()
	TObjectPtr<UTacTurnSubsystem> TurnSubsystem = nullptr;
};

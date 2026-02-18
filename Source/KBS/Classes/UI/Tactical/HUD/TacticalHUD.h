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
class UCanvasPanel;
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

	// Canvas for panel contents (non-popup widgets)
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UCanvasPanel* PanelCanvas;

	// Overlay for popups (modal widgets on top)
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UOverlay* HUDLayout;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UUnitDetailsPopup> UnitDetailsPopup;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UUnitSpellbookPopup> SpellbookPopup;

	// Panel widgets bound from designer
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTurnQueuePanel> TurnQueuePanel;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTurnCounterLabel> TurnCounterLabel;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTeamPanel> TeamPanel;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UCurrentUnitPanel> CurrentUnitPanel;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UHoveredUnitPanel> HoveredUnitPanel;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UAbilityPanel> AbilityPanel;

private:
	UFUNCTION()
	void OnUnitDetailsPopupCloseRequested();

	UFUNCTION()
	void OnSpellbookPopupCloseRequested();

	UFUNCTION()
	void HandleSpellbookAbilitySelected(UUnitAbilityInstance* Ability);

	UFUNCTION()
	void HandleAbilityPanelAbilitySelected(UUnitAbilityInstance* Ability);

	UPROPERTY()
	TObjectPtr<UTacTurnSubsystem> TurnSubsystem = nullptr;
};

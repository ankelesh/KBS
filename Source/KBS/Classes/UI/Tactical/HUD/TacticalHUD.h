#pragma once
#include "UI/Common/HUD/KbsHUD.h"
#include "TacticalHUD.generated.h"

class UUnitDetailsPopup;
class UOverlay;
class AUnit;

UCLASS(Blueprintable)
class KBS_API UTacticalHUD : public UKbsHUD
{
	GENERATED_BODY()

public:
	// Show unit details popup modally
	UFUNCTION(BlueprintCallable, Category = "Tactical HUD")
	void ShowUnitDetailsPopup(AUnit* Unit);

	// Hide unit details popup
	UFUNCTION(BlueprintCallable, Category = "Tactical HUD")
	void HideUnitDetailsPopup();

protected:
	virtual void NativeConstruct() override;

	// Overlay for managing Z-ordering (popup always on top)
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UOverlay* PopupOverlay;

	// Widget class for unit details popup
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UUnitDetailsPopup> UnitDetailsPopupClass;

private:
	UFUNCTION()
	void OnUnitDetailsPopupCloseRequested();

	// Cached popup instance (created on first use, hidden by default)
	UPROPERTY()
	TObjectPtr<UUnitDetailsPopup> UnitDetailsPopup;
};

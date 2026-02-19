#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitTeamSlot.generated.h"

class AUnit;
class UImage;
class UTextBlock;
class UButton;

// Light, self-refreshable view of a unit's state for team display
// Displays portrait, frame, name, and HP overlay; listens to unit events for updates
UCLASS(Blueprintable)
class KBS_API UUnitTeamSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set up widget with unit reference (binds to unit events, refreshes display)
	UFUNCTION(BlueprintCallable, Category = "Unit Team Slot")
	void SetupUnit(AUnit* InUnit);

	// Clean widget state for pooling/reuse (unbinds events, clears data, hides)
	UFUNCTION(BlueprintCallable, Category = "Unit Team Slot")
	void Clear();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// UI Components
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImage* Portrait;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImage* Frame;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* Name;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImage* HPOverlay;

	// Visual properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor OverlayColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor OverlayDeathColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor AttackerFrameColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor DefenderFrameColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange tint

	UPROPERTY(BlueprintReadOnly, Category = "State")
	TWeakObjectPtr<AUnit> BoundUnit = nullptr;

private:
	// Event handlers
	UFUNCTION()
	void OnUnitHealthChange(AUnit* Owner, int32 NewHealth);

	UFUNCTION()
	void OnUnitDied(AUnit* Unit);

	// Visual update methods
	void RefreshDisplay();
	void UpdateHPOverlay();
	void UpdateFrameColor();
	void SetHPOverlayHeight(float HealthPercent);
	void ApplyDeathVisuals();

	// Helper to get TacticalHUD from player controller
	class UTacticalHUD* GetTacticalHUD() const;

public:
	// Blueprint hooks for custom visual effects (empty by default, override to add styling)
	UFUNCTION(BlueprintImplementableEvent, Category = "Unit Team Slot|Visuals")
	void BP_OnUnitDamaged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Unit Team Slot|Visuals")
	void BP_OnUnitDied();
};

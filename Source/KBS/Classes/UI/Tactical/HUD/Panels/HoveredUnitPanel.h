#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HoveredUnitPanel.generated.h"

class AUnit;
class UBattleEffectSlotSnapshot;
class UTacticalHUD;
class UImage;
class UTextBlock;
class UHorizontalBox;
class UButton;

// Panel displaying hovered unit's portrait, stats snapshot, and effect snapshots
// Passive/read-only: reinits when hovered unit changes, does not listen to unit events
UCLASS(Blueprintable)
class KBS_API UHoveredUnitPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set currently hovered unit (reinits if different from current)
	UFUNCTION(BlueprintCallable, Category = "Hovered Unit Panel")
	void SetHoveredUnit(AUnit* Unit);

	// Clean widget and clear hovered unit
	UFUNCTION(BlueprintCallable, Category = "Hovered Unit Panel")
	void Clear();

	// Blueprint hook for visual events
	UFUNCTION(BlueprintImplementableEvent, Category = "Hovered Unit Panel|Events")
	void BP_OnHoveredUnitChanged(AUnit* NewUnit);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI Components - Portrait Section
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UButton* PortraitButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImage* UnitPortraitImage;

	// UI Components - Stats Section
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* UnitNameText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* InitiativeText;

	// UI Components - Effect Panel
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UHorizontalBox* EffectSnapshotContainer;

	// Widget class for effect snapshots
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UBattleEffectSlotSnapshot> BattleEffectSlotSnapshotClass;

private:
	UFUNCTION()
	void OnPortraitRightClicked();

	void RefreshPanel(AUnit* Unit);
	void RefreshPortrait(AUnit* Unit);
	void RefreshStats(AUnit* Unit);
	void RefreshEffectSnapshots(AUnit* Unit);

	UPROPERTY()
	TObjectPtr<AUnit> HoveredUnit = nullptr;

	UPROPERTY()
	TObjectPtr<UTacticalHUD> OwningHUD = nullptr;

	// Cached effect snapshot slots (max 5, create as needed)
	UPROPERTY()
	TArray<TObjectPtr<UBattleEffectSlotSnapshot>> EffectSnapshots;

	static constexpr int32 MAX_EFFECT_SNAPSHOTS = 5;
};

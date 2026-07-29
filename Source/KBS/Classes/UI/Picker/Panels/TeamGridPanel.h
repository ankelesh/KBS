#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeamGridPanel.generated.h"

class UUnitDefinition;
class UTeamUnitSlot;
class UGridPanel;

UCLASS(Blueprintable)
class KBS_API UTeamGridPanel : public UUserWidget
{
	GENERATED_BODY()
public:
	// Refresh slot visuals from the HUD's authoritative slot data (6 elements: [Row*2+Col])
	void RefreshFromSlots(const TArray<TObjectPtr<UUnitDefinition>>& Slots);

	// Called by TeamUnitSlot on drop/clear — routed to PickerHUD
	void NotifyDrop(UUnitDefinition* Definition, int32 Row, int32 Col);
	void NotifyClear(int32 Row, int32 Col);

	DECLARE_DELEGATE_ThreeParams(FOnUnitDropped, UUnitDefinition*, int32, int32);
	FOnUnitDropped OnUnitDropped;

	DECLARE_DELEGATE_TwoParams(FOnSlotCleared, int32, int32);
	FOnSlotCleared OnSlotCleared;

protected:
	virtual void NativeConstruct() override;

	// Designer places a UGridPanel named SlotGrid (3 rows x 2 cols) in the BP
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UGridPanel> SlotGrid;
	UPROPERTY(EditDefaultsOnly) TSubclassOf<UTeamUnitSlot> SlotClass;

private:
	static constexpr int32 Rows = 3;
	static constexpr int32 Cols = 2;

	TObjectPtr<UTeamUnitSlot> GetSlot(int32 Row, int32 Col) const;

	UPROPERTY() TArray<TObjectPtr<UTeamUnitSlot>> CreatedSlots; // [Row*Cols+Col]
};

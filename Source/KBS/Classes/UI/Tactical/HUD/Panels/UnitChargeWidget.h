#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "UnitChargeWidget.generated.h"

class AUnit;
class UUnitChargeComponent;
class UUnitChargeSlot;
class UHorizontalBox;
class UTacTurnSubsystem;

// Wraps all charge counters for the current unit.
// Pops up on turn start if the unit has UUnitChargeComponent; hides otherwise.
// Listens to OnChargeChanged to refresh individual slots in real time.
UCLASS(Blueprintable)
class KBS_API UUnitChargeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Unit Charge Widget")
	void Clear();

	// --- BP hooks for visual events ---

	UFUNCTION(BlueprintImplementableEvent, Category = "Unit Charge Widget|Events")
	void BP_OnUnitSet(AUnit* NewUnit);

	// Called when unit has no charge component — hide your root here
	UFUNCTION(BlueprintImplementableEvent, Category = "Unit Charge Widget|Events")
	void BP_OnHidden();

	// Called after all slots are rebuilt
	UFUNCTION(BlueprintImplementableEvent, Category = "Unit Charge Widget|Events")
	void BP_OnCountersRefreshed(int32 CounterCount);

	// Called each time a single charge value changes
	UFUNCTION(BlueprintImplementableEvent, Category = "Unit Charge Widget|Events")
	void BP_OnChargeChanged(const FGameplayTag& Tag, int32 NewValue);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Container — bind in UMG designer
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> ChargeSlotContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UUnitChargeSlot> ChargeSlotClass;

private:
	UFUNCTION()
	void OnTurnStarted(AUnit* Unit);

	UFUNCTION()
	void OnChargeChangedHandler(const FGameplayTag& Tag, int32 NewValue);

	void SetupForUnit(AUnit* Unit);
	void RebuildSlots(UUnitChargeComponent* Component);
	void UnbindFromCurrentComponent();

	UPROPERTY()
	TObjectPtr<AUnit> CurrentUnit = nullptr;

	UPROPERTY()
	TObjectPtr<UUnitChargeComponent> BoundComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UTacTurnSubsystem> TurnSubsystem = nullptr;

	// Tag -> slot map for O(1) updates on OnChargeChanged
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UUnitChargeSlot>> SlotMap;
};

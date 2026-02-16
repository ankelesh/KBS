#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnQueuePanel.generated.h"

class AUnit;
class UUnitTurnQueueSlot;
class UVerticalBox;
class UTacticalHUD;
class UTacTurnSubsystem;

// Panel displaying turn queue: current unit slot + up to 10 upcoming units
// Updates on TurnSubsystem OnTurnStart event
UCLASS(Blueprintable)
class KBS_API UTurnQueuePanel : public UUserWidget
{
	GENERATED_BODY()

public:
	// Clean widget and clear all slots
	UFUNCTION(BlueprintCallable, Category = "Turn Queue Panel")
	void Clear();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI Components - Current Unit Section
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UVerticalBox* CurrentUnitContainer;

	// UI Components - Queue Section
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UVerticalBox* QueueContainer;

	// Widget class for turn queue slots
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UUnitTurnQueueSlot> UnitTurnQueueSlotClass;

private:
	UFUNCTION()
	void OnTurnStart(AUnit* Unit);

	UFUNCTION()
	void OnUnitDetailsRequested(AUnit* Unit);

	void UpdatePanel();
	void UpdateCurrentUnitSlot(AUnit* CurrentUnit);
	void UpdateQueueSlots(const TArray<AUnit*>& RemainingUnits);

	UPROPERTY()
	TObjectPtr<UTacTurnSubsystem> TurnSubsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UTacticalHUD> OwningHUD = nullptr;

	// Cached current unit slot (1 slot)
	UPROPERTY()
	TObjectPtr<UUnitTurnQueueSlot> CurrentUnitSlot = nullptr;

	// Cached queue slots (10 slots, pre-allocated)
	UPROPERTY()
	TArray<TObjectPtr<UUnitTurnQueueSlot>> QueueSlots;

	static constexpr int32 MAX_QUEUE_SLOTS = 10;
};

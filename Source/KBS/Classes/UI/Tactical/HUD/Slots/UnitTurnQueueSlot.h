#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitTurnQueueSlot.generated.h"

class AUnit;
struct FUnitTurnQueueDisplay;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitDetailsRequested, AUnit*, Unit);

// Represents a unit in the turn order queue, displays portrait, name, and rolled initiative
// Managed externally - does not listen to events. Right-click emits OnDetailsRequested.
UCLASS(Blueprintable)
class KBS_API UUnitTurnQueueSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitDetailsRequested OnDetailsRequested;

	// Setup slot with display data, rolled initiative, and unit reference (for right-click callback)
	UFUNCTION(BlueprintCallable, Category = "Unit Turn Queue")
	void SetupSlot(const FUnitTurnQueueDisplay& DisplayData, int32 RolledInitiative, AUnit* Unit);

	// Clear all widget contents (empties text, clears portrait, resets border)
	UFUNCTION(BlueprintCallable, Category = "Unit Turn Queue")
	void Clear();

	// Reset widget to clean state for pooling/reuse (clears + unbinds)
	UFUNCTION(BlueprintCallable, Category = "Unit Turn Queue")
	void Clean();

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// UI Components
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UBorder* SlotBorder;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImage* Portrait;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* UnitName;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* InitiativeLabel;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AUnit> TrackedUnit = nullptr;

public:
	// Blueprint hooks for custom visual styling (empty by default, override to add styling)
	UFUNCTION(BlueprintImplementableEvent, Category = "Unit Turn Queue|Visuals")
	void BP_OnSetupSlot(const FUnitTurnQueueDisplay& DisplayData, int32 RolledInitiative);

	UFUNCTION(BlueprintImplementableEvent, Category = "Unit Turn Queue|Visuals")
	void BP_OnClear();

	UFUNCTION(BlueprintImplementableEvent, Category = "Unit Turn Queue|Visuals")
	void BP_OnClean();
};

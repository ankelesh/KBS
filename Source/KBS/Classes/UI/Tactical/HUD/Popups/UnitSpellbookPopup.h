#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitSpellbookPopup.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogKBSUI, Log, All);

class AUnit;
class UActiveAbilitySlot;
class UUnitAbilityInstance;
class UButton;
class UTextBlock;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilitySelected, UUnitAbilityInstance*, Ability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpellbookPopupOnCloseRequired);

// Modal popup displaying spellbook abilities from AbilityInventoryComponent
// Hidden by default, shows clickable ability slots with multiclick protection
// Closes on ability selection or quit button click
UCLASS(Blueprintable)
class KBS_API UUnitSpellbookPopup : public UUserWidget
{
	GENERATED_BODY()

public:
	// Event fired when popup should close (quit button or ability selected)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FSpellbookPopupOnCloseRequired OnCloseRequired;

	// Event fired when an ability is selected from the spellbook
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAbilitySelected OnAbilitySelected;

	// Setup popup with unit's spellbook (re-reads inventory, regenerates slots)
	UFUNCTION(BlueprintCallable, Category = "Spellbook Popup")
	void SetupFromUnit(AUnit* Unit);

	// Clear popup and reset to empty state (hides slots, doesn't destroy them)
	UFUNCTION(BlueprintCallable, Category = "Spellbook Popup")
	void Clear();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// UI Components
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UVerticalBox* AbilityListContainer;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* TitleText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UButton* CloseButton;

	// Widget Classes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UActiveAbilitySlot> AbilitySlotClass;

	// Pool Settings (reserves 10 slots, expands if needed)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pool Settings")
	int32 InitialAbilitySlots = 10;

	// Default title text
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Settings")
	FText DefaultTitleText = FText::FromString(TEXT("Spellbook"));

private:
	void InitializePools();
	void PopulateAbilities(AUnit* Unit);
	void ResetAbilitySlots();

	UActiveAbilitySlot* GetOrCreateAbilitySlot();

	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	void OnAbilitySlotClicked(UUnitAbilityInstance* Ability);

	// Widget pool (never shrinks, only grows as needed)
	UPROPERTY()
	TArray<TObjectPtr<UActiveAbilitySlot>> AbilitySlotPool;

	int32 AbilitySlotsInUse = 0;

	// Multiclick protection
	bool bIsProcessingClick = false;
};

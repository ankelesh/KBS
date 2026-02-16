#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpellbookSlot.generated.h"

class AUnit;
class UButton;
class UActiveAbilitySlotSnapshot;
class UUnitAbilityInstance;

// Lightweight shortcut to unit's spellbook - button to open popup + selected spell display
// Hides entirely if unit has no spellbook available
UCLASS(Blueprintable)
class KBS_API USpellbookSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set up widget with unit reference (checks spellbook availability, binds to popup events)
	UFUNCTION(BlueprintCallable, Category = "Spellbook Slot")
	void SetupUnit(AUnit* InUnit);

	// Clean widget state (unbinds events, clears data, hides)
	UFUNCTION(BlueprintCallable, Category = "Spellbook Slot")
	void Clear();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI Components
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UButton* SpellbookButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UActiveAbilitySlotSnapshot* SpellbookSpellSlot;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	TWeakObjectPtr<AUnit> BoundUnit = nullptr;

private:
	UFUNCTION()
	void OnSpellbookButtonClicked();

	UFUNCTION()
	void OnSpellbookAbilitySelected(UUnitAbilityInstance* Ability);

	bool bDelegatesBound = false;
};

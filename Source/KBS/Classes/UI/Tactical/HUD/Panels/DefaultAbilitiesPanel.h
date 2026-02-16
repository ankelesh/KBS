#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DefaultAbilitiesPanel.generated.h"

class AUnit;
class UActiveAbilitySlot;
class UUnitAbilityInstance;
class UHorizontalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDefaultAbilitiesPanelOnAbilitySelected, UUnitAbilityInstance*, Ability);

// Panel for default unit abilities (Attack, Move, Wait, Defend, Flee)
// Transits child slot selection events to parent, manages 5 fixed ability slots
UCLASS(Blueprintable)
class KBS_API UDefaultAbilitiesPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FDefaultAbilitiesPanelOnAbilitySelected OnAbilitySelected;

	// Clean all slots to empty state
	UFUNCTION(BlueprintCallable, Category = "Default Abilities Panel")
	void Clear();

	// Fill slots with unit's default abilities (Attack, Move, Wait, Defend, Flee)
	UFUNCTION(BlueprintCallable, Category = "Default Abilities Panel")
	void SetUnit(AUnit* Unit);

	// Select ability in matching slot (deselects all others)
	UFUNCTION(BlueprintCallable, Category = "Default Abilities Panel")
	void SelectAbility(UUnitAbilityInstance* Ability);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI Components
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UHorizontalBox> SlotsContainer;

	// Widget class for ability slots
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UActiveAbilitySlot> ActiveAbilitySlotClass;

private:
	UFUNCTION()
	void OnChildAbilitySelected(UUnitAbilityInstance* Ability);

	void CreateSlots();

	// Cached slots (5 slots for Attack, Move, Wait, Defend, Flee)
	UPROPERTY()
	TObjectPtr<UActiveAbilitySlot> AttackSlot;

	UPROPERTY()
	TObjectPtr<UActiveAbilitySlot> MoveSlot;

	UPROPERTY()
	TObjectPtr<UActiveAbilitySlot> WaitSlot;

	UPROPERTY()
	TObjectPtr<UActiveAbilitySlot> DefendSlot;

	UPROPERTY()
	TObjectPtr<UActiveAbilitySlot> FleeSlot;

	static constexpr int32 REQUIRED_SLOTS = 5;
};

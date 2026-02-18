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

	// Ability slots bound from Blueprint (5 slots for Attack, Move, Wait, Defend, Flee)
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UActiveAbilitySlot> AttackSlot;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UActiveAbilitySlot> MoveSlot;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UActiveAbilitySlot> WaitSlot;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UActiveAbilitySlot> DefendSlot;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UActiveAbilitySlot> FleeSlot;

private:
	UFUNCTION()
	void OnChildAbilitySelected(UUnitAbilityInstance* Ability);
};

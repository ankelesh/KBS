#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Tactical/HUD/Slots/IconTypes.h"
#include "ActiveAbilitySlot.generated.h"

struct FAbilityDisplayData;
class UUnitAbilityInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActiveAbilitySlotOnAbilitySelected, UUnitAbilityInstance*, Ability);

// State-displaying representation of UnitAbilityInstance, clickable brick for larger panels
// States: Enabled (available, clickable), Disabled (visible, greyed out), Hidden (collapsed)
UCLASS(Blueprintable)
class KBS_API UActiveAbilitySlot : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FActiveAbilitySlotOnAbilitySelected OnAbilitySelected;

	// Replace current ability (unbinds old, binds new, checks CanExecute for initial state)
	void SetAbility(UUnitAbilityInstance* NewAbility);

	// Bind to ability without replacing display state
	void BindToAbility(UUnitAbilityInstance* Ability);

	// Reset widget to clean state for pooling/reuse (unbinds, hides, clears data)
	void Clean();

	// State transitions
	void Select();   // Enabled → Selected
	void Deselect(); // Selected → Enabled
	void Enable();   // Disabled → Enabled
	void Disable();  // Enabled/Selected → Disabled
	void Hide();     // Any → Hidden

	bool HasAbility(UUnitAbilityInstance* Ability) const;
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI Components
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class USizeBox* SizeBox;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UOverlay* SlotRoot;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UBorder* SelectionBorder;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImage* AbilityIcon;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* Charges;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UButton* SlotButton;
	
	UPROPERTY(BlueprintReadWrite)
	FLinearColor SelectedColor;
		
	UPROPERTY(BlueprintReadWrite)
	FLinearColor DefaultColor;

	UPROPERTY(BlueprintReadOnly)
	EAbilitySlotState CurrentState = EAbilitySlotState::Hidden;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UUnitAbilityInstance> BoundAbility = nullptr;

private:
	UFUNCTION()
	void OnSlotButtonClicked(); // Broadcasts OnAbilitySelected

	UFUNCTION()
	void OnAbilityAvailabilityChanged(const UUnitAbilityInstance* Ability, bool bAvailable);

	UFUNCTION()
	void OnAbilityUsed(int32 ChargesLeft, bool bAvailable);

	void UpdateChargesDisplay(); // Shows charges if Ability->HasExplicitCharges() is true

	// Visual state application - C++ handles essential state, calls BP hooks for styling
	void ApplySelectedVisuals();
	void ApplyEnabledVisuals();
	void ApplyDisabledVisuals();
	void ApplyHiddenVisuals();

public:
	// Blueprint hooks for custom visual styling (empty by default, override to add styling)
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability Slot|Visuals")
	void BP_OnApplySelectedVisuals();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability Slot|Visuals")
	void BP_OnApplyEnabledVisuals();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability Slot|Visuals")
	void BP_OnApplyDisabledVisuals();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability Slot|Visuals")
	void BP_OnApplyHiddenVisuals();
};

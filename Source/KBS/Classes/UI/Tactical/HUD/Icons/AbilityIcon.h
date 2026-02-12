#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Tactical/HUD/Icons/IconTypes.h"
#include "AbilityIcon.generated.h"

struct FAbilityDisplayData;
class UUnitAbilityInstance;

UCLASS()
class KBS_API UAbilityIcon : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetAbility(const FAbilityDisplayData& DisplayData);
	void BindToAbility(UUnitAbilityInstance* Ability);

protected:
	virtual void NativeConstruct() override;

	// UI Components
	UPROPERTY()
	class USizeBox* SizeBox;

	UPROPERTY()
	class UOverlay* SlotRoot;

	UPROPERTY()
	class UBorder* SelectionBorder;

	UPROPERTY()
	class UImage* AbilityIcon;

	UPROPERTY()
	class UTextBlock* Charges;

	UPROPERTY()
	class UButton* SlotButton;

	// Cached display state
	EIconState CurrentState = EIconState::IconEmpty;
	int32 CachedRemainingCharges = 0;
	int32 CachedMaxCharges = 0;

private:
	UFUNCTION()
	void OnSlotButtonClicked();

	UFUNCTION()
	void OnAbilityAvailabilityChanged(const UUnitAbilityInstance* Ability, bool bAvailable);

	void ApplyVisualState();
	void UpdateChargesDisplay();
};

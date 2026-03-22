#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "UnitChargeSlot.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

// Displays a single charge counter: icon + current value
UCLASS(Blueprintable)
class KBS_API UUnitChargeSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupSlot(const FGameplayTag& Tag, int32 Value, TSoftObjectPtr<UTexture2D> Icon);
	void UpdateValue(int32 NewValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Charge Slot|Events")
	void BP_OnSetup(const FGameplayTag& Tag, int32 Value);

	UFUNCTION(BlueprintImplementableEvent, Category = "Charge Slot|Events")
	void BP_OnValueChanged(int32 NewValue);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> ChargeIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ChargeValueText;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag BoundTag;
};

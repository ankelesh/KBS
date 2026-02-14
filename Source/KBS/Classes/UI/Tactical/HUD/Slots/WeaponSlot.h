#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponSlot.generated.h"

class UWeapon;
class UTextBlock;
class UHorizontalBox;

// Display widget for weapon information
// Shows weapon name, damage, damage types, and effects
UCLASS(Blueprintable)
class KBS_API UWeaponSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	// Setup from weapon data
	UFUNCTION(BlueprintCallable, Category = "Weapon Slot")
	void SetupFromWeapon(UWeapon* Weapon, class AUnit* Owner);

	// Clear and hide widget
	UFUNCTION(BlueprintCallable, Category = "Weapon Slot")
	void Clear();

protected:
	// UI Components
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* WeaponNameText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* DamageText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* DamageTypesText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* TargetTypeText;

	// Container for effect icons (optional)
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UHorizontalBox* EffectIconList;

	// Widget class for effect icons
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<class UBattleEffectSlotSnapshot> EffectSlotClass;

private:
	// Store created effect slots for cleanup
	UPROPERTY()
	TArray<TObjectPtr<class UBattleEffectSlotSnapshot>> CreatedEffectSlots;
};

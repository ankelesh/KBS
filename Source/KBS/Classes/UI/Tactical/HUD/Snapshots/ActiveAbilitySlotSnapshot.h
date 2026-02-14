#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActiveAbilitySlotSnapshot.generated.h"

class UUnitAbilityInstance;
class UImage;
class UTextBlock;

// Lightweight read-only display of an active ability - no event binding, no state tracking
// Shows icon, charges, and tooltip only
UCLASS(Blueprintable)
class KBS_API UActiveAbilitySlotSnapshot : public UUserWidget
{
	GENERATED_BODY()

public:
	// Setup from ability data (read-only, no event binding)
	UFUNCTION(BlueprintCallable, Category = "Ability Slot Snapshot")
	void SetupFromAbility(UUnitAbilityInstance* Ability);

	// Clear and hide widget
	UFUNCTION(BlueprintCallable, Category = "Ability Slot Snapshot")
	void Clear();

protected:
	// UI Components
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImage* AbilityIcon;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* ChargesText;
};

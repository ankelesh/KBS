#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PassiveAbilitySlotSnapshot.generated.h"

class UUnitAbility;
class UImage;

// Lightweight read-only display of a passive ability - no event binding
// Shows icon and tooltip only
UCLASS(Blueprintable)
class KBS_API UPassiveAbilitySlotSnapshot : public UUserWidget
{
	GENERATED_BODY()

public:
	// Setup from ability data (read-only, no event binding)
	UFUNCTION(BlueprintCallable, Category = "Passive Ability Slot Snapshot")
	void SetupFromAbility(UUnitAbility* Ability);

	// Clear and hide widget
	UFUNCTION(BlueprintCallable, Category = "Passive Ability Slot Snapshot")
	void Clear();

protected:
	// UI Components
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImage* AbilityIcon;
};

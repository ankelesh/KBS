#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PassiveAbilitySlot.generated.h"

struct FAbilityDisplayData;
class UUnitAbilityInstance;

// Simple icon display for passive abilities - flashes VFX when triggered
// Supports pure visual mode (no event binding) for static menus
UCLASS(Blueprintable)
class KBS_API UPassiveAbilitySlot : public UUserWidget
{
	GENERATED_BODY()

public:
	// Setup widget with ability data. If bBindToEvents=true, listens to OnAbilityUsed for VFX triggers
	UFUNCTION(BlueprintCallable, Category = "Passive Ability Slot")
	void SetupFromAbility(UUnitAbilityInstance* Ability, bool bBindToEvents = true);

	// Reset widget to clean state for pooling/reuse (unbinds, hides, clears data)
	UFUNCTION(BlueprintCallable, Category = "Passive Ability Slot")
	void Clear();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI Components
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImage* AbilityIcon;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UUnitAbilityInstance> BoundAbility = nullptr;

	UPROPERTY(BlueprintReadOnly)
	bool bIsListeningToEvents = false;

private:
	UFUNCTION()
	void OnAbilityTriggered(int32 ChargesLeft, bool bAvailable);

public:
	// Blueprint hook called when passive ability triggers - implement to show flash VFX
	// Only fires if bBindToEvents was true in SetupFromAbility
	UFUNCTION(BlueprintImplementableEvent, Category = "Passive Ability Slot|Events")
	void BP_OnAbilityTriggered(UUnitAbilityInstance* Ability);
};

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CurrentUnitPanel.generated.h"

class AUnit;
class UBattleEffectSlot;
class UPassiveAbilitySlot;
class UTacTurnSubsystem;
class UTacticalHUD;
class UImage;
class UTextBlock;
class UHorizontalBox;
class UButton;

// Panel displaying currently active unit's portrait, stats, effects, and passive abilities
// Listens to TurnSubsystem's OnTurnStart, refreshes when new unit's turn begins
UCLASS(Blueprintable)
class KBS_API UCurrentUnitPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	// Clean widget and unbind from current unit
	UFUNCTION(BlueprintCallable, Category = "Current Unit Panel")
	void Clear();

	// Blueprint hooks for visual events
	UFUNCTION(BlueprintImplementableEvent, Category = "Current Unit Panel|Events")
	void BP_OnUnitChanged(AUnit* NewUnit);

	UFUNCTION(BlueprintImplementableEvent, Category = "Current Unit Panel|Events")
	void BP_OnHealthChanged(int32 CurrentHealth, int32 MaxHealth);

	UFUNCTION(BlueprintImplementableEvent, Category = "Current Unit Panel|Events")
	void BP_OnEffectsRefreshed(int32 EffectCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Current Unit Panel|Events")
	void BP_OnPassiveAbilitiesRefreshed(int32 AbilityCount);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI Components - Portrait Section
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UButton* PortraitButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImage* UnitPortraitImage;

	// UI Components - Stats Section
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* UnitNameText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* InitiativeText;

	// UI Components - Effect Panel
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UHorizontalBox* EffectSlotContainer;

	// UI Components - Passive Ability Panel
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UHorizontalBox* PassiveAbilitySlotContainer;

	// Widget classes for slots
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UBattleEffectSlot> BattleEffectSlotClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UPassiveAbilitySlot> PassiveAbilitySlotClass;

private:
	UFUNCTION()
	void OnTurnStarted(AUnit* Unit);

	UFUNCTION()
	void OnHealthChanged(AUnit* Unit, int32 NewHealth);

	UFUNCTION()
	void OnPortraitClicked();

	void RefreshPanel(AUnit* Unit);
	void RefreshPortrait(AUnit* Unit);
	void RefreshStats(AUnit* Unit);
	void RefreshEffects(AUnit* Unit);
	void RefreshPassiveAbilities(AUnit* Unit);

	void UnbindFromCurrentUnit();

	UPROPERTY()
	TObjectPtr<AUnit> CurrentUnit = nullptr;

	UPROPERTY()
	TObjectPtr<UTacTurnSubsystem> TurnSubsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UTacticalHUD> OwningHUD = nullptr;

	// Cached effect slots (max 5, don't reserve, cache as needed)
	UPROPERTY()
	TArray<TObjectPtr<UBattleEffectSlot>> EffectSlots;

	// Cached passive ability slots (reserve 2, cache additions up to 5)
	UPROPERTY()
	TArray<TObjectPtr<UPassiveAbilitySlot>> PassiveAbilitySlots;

	static constexpr int32 MAX_EFFECT_SLOTS = 5;
	static constexpr int32 MAX_PASSIVE_SLOTS = 5;
	static constexpr int32 RESERVED_PASSIVE_SLOTS = 2;
};

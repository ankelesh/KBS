#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitDetailsPopup.generated.h"

class AUnit;
class UImage;
class UTextBlock;
class UVerticalBox;
class UHorizontalBox;
class UActiveAbilitySlotSnapshot;
class UPassiveAbilitySlotSnapshot;
class UBattleEffectSlotSnapshot;
class UWeaponSlot;



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUnitDetailsPopupOnCloseRequired);

// Modal popup displaying comprehensive unit information
// Hidden by default, shows portrait, stats, weapons, abilities, and effects
// Uses widget pooling for performance - pre-creates slots and reuses them
UCLASS(Blueprintable)
class KBS_API UUnitDetailsPopup : public UUserWidget
{
	GENERATED_BODY()

public:
	// Event fired when popup should close (e.g., right-click)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FUnitDetailsPopupOnCloseRequired OnCloseRequired;

	// Setup popup with unit data (BP invokable)
	UFUNCTION(BlueprintCallable, Category = "Unit Details Popup")
	void SetupFromUnit(AUnit* Unit);

	// Clear popup and reset to empty state (hides unused slots, doesn't destroy them)
	UFUNCTION(BlueprintCallable, Category = "Unit Details Popup")
	void Clear();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// UI Components - Portrait
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImage* PortraitImage;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* UnitNameText;

	// UI Components - Stats
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* CurrentHealthText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* MaxHealthText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* InitiativeText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* AccuracyText;

	// UI Components - Defence
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* ArmourText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* WardsText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* ImmunitiesText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* DamageReductionText;

	// UI Components - Status
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* StatusText;

	// UI Components - Weapon Containers
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UVerticalBox* WeaponList;

	// UI Components - Ability Containers
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UHorizontalBox* ActiveAbilityList;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UHorizontalBox* PassiveAbilityList;

	// UI Components - Effect Container
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UHorizontalBox* EffectList;

	// Widget Classes for dynamic spawning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UWeaponSlot> WeaponSlotClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UActiveAbilitySlotSnapshot> ActiveAbilitySlotClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UPassiveAbilitySlotSnapshot> PassiveAbilitySlotClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UBattleEffectSlotSnapshot> BattleEffectSlotClass;

	// Initial pool sizes (pre-allocated on construct)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pool Settings")
	int32 InitialWeaponSlots = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pool Settings")
	int32 InitialActiveAbilitySlots = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pool Settings")
	int32 InitialPassiveAbilitySlots = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pool Settings")
	int32 InitialEffectSlots = 5;

private:
	void InitializePools();
	void PopulateStats(AUnit* Unit);
	void PopulateWeapons(AUnit* Unit);
	void PopulateAbilities(AUnit* Unit);
	void PopulateEffects(AUnit* Unit);
	void PopulateStatus(AUnit* Unit);

	void ResetWeaponSlots();
	void ResetAbilitySlots();
	void ResetEffectSlots();

	// Pool management - get slot from pool or create new one
	UWeaponSlot* GetOrCreateWeaponSlot();
	UActiveAbilitySlotSnapshot* GetOrCreateActiveAbilitySlot();
	UPassiveAbilitySlotSnapshot* GetOrCreatePassiveAbilitySlot();
	UBattleEffectSlotSnapshot* GetOrCreateEffectSlot();

	// Widget pools (never shrink, only grow as needed)
	UPROPERTY()
	TArray<TObjectPtr<UWeaponSlot>> WeaponSlotPool;

	UPROPERTY()
	TArray<TObjectPtr<UActiveAbilitySlotSnapshot>> ActiveAbilitySlotPool;

	UPROPERTY()
	TArray<TObjectPtr<UPassiveAbilitySlotSnapshot>> PassiveAbilitySlotPool;

	UPROPERTY()
	TArray<TObjectPtr<UBattleEffectSlotSnapshot>> EffectSlotPool;

	// Track how many slots are currently in use
	int32 WeaponSlotsInUse = 0;
	int32 ActiveAbilitySlotsInUse = 0;
	int32 PassiveAbilitySlotsInUse = 0;
	int32 EffectSlotsInUse = 0;
};

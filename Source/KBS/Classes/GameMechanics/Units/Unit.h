#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "UnitStats.h"
#include "Unit.generated.h"

class UWeapon;
class UUnitDefinition;
class UBattleEffectComponent;
class UBattleEffect;
class UAbilityInventoryComponent;
class UUnitVisualsComponent;
struct FDamageResult;

UENUM(BlueprintType)
enum class EBattleLayer : uint8
{
	Ground = 0 UMETA(DisplayName = "Ground"),
	Air = 1 UMETA(DisplayName = "Air")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitClicked, AUnit*, ClickedUnit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitAttacked, AUnit*, Victim, AUnit*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitDamaged, AUnit*, Victim, AUnit*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitAttacks, AUnit*, Attacker, AUnit*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitDied, AUnit*, Unit);

UCLASS()
class KBS_API AUnit : public APawn
{
	GENERATED_BODY()

public:
	AUnit();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitClicked OnUnitClicked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitAttacked OnUnitAttacked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitDamaged OnUnitDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitAttacks OnUnitAttacks;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitDied OnUnitDied;

	UFUNCTION(BlueprintCallable, Category = "Display")
	FUnitDisplayData GetDisplayData() const;

	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;

	void RecalculateModifiedStats();
	void RecalculateAllWeaponStats();
	void Restore();
	void LevelUp();
	void SetUnitDefinition(UUnitDefinition* InDefinition);

	const FUnitCoreStats& GetStats() const { return ModifiedStats; }
	float GetCurrentHealth() const { return CurrentHealth; }
	const FUnitProgressionData& GetProgression() const { return Progression; }
	const TArray<TObjectPtr<UWeapon>>& GetWeapons() const { return Weapons; }
	

	void TakeHit(const FDamageResult& DamageResult);
	void ApplyEffect(UBattleEffect* Effect);

	// Turn lifecycle hooks
	UFUNCTION()
	void OnUnitTurnStart();

	UFUNCTION()
	void OnUnitTurnEnd();

	// Stat accessors
	const FUnitCoreStats& GetModifiedStats() const { return ModifiedStats; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UUnitVisualsComponent> VisualsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBattleEffectComponent> EffectManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilityInventoryComponent> AbilityInventory;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
	TObjectPtr<UUnitDefinition> UnitDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
	int32 InitialLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FUnitCoreStats BaseStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	FUnitCoreStats ModifiedStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FUnitProgressionData Progression;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<UWeapon>> Weapons;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitStats.h"
#include "Unit.generated.h"

class UWeapon;
class UUnitDefinition;
class UEffectManagerComponent;

UENUM(BlueprintType)
enum class EBattleLayer : uint8
{
	Ground = 0 UMETA(DisplayName = "Ground"),
	Air = 1 UMETA(DisplayName = "Air")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitClicked, AUnit*, ClickedUnit);

UCLASS()
class KBS_API AUnit : public AActor
{
	GENERATED_BODY()

public:
	AUnit();

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitClicked OnUnitClicked;

	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;

	void RecalculateModifiedStats();
	void RecalculateAllWeaponStats();
	void Restore();
	void LevelUp();

	const FUnitCoreStats& GetStats() const { return ModifiedStats; }
	float GetCurrentHealth() const { return CurrentHealth; }
	const FUnitProgressionData& GetProgression() const { return Progression; }
	const TArray<TObjectPtr<UWeapon>>& GetWeapons() const { return Weapons; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEffectManagerComponent> EffectManager;

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

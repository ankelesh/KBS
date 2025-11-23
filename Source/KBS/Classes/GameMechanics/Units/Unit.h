#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitStats.h"
#include "Unit.generated.h"

class UWeapon;

UENUM(BlueprintType)
enum class EBattleLayer : uint8
{
	Ground = 0 UMETA(DisplayName = "Ground"),
	Air = 1 UMETA(DisplayName = "Air")
};

UCLASS()
class KBS_API AUnit : public AActor
{
	GENERATED_BODY()

public:
	AUnit();

	int32 GetGridRow() const { return GridRow; }
	int32 GetGridCol() const { return GridCol; }
	EBattleLayer GetGridLayer() const { return GridLayer; }

	void SetGridPosition(int32 Row, int32 Col, EBattleLayer Layer);

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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 GridRow = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 GridCol = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	EBattleLayer GridLayer = EBattleLayer::Ground;

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

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitClicked, AUnit*, ClickedUnit);

UCLASS()
class KBS_API AUnit : public AActor
{
	GENERATED_BODY()

public:
	AUnit();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitClicked OnUnitClicked;

	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;

	int32 GetGridRow() const { return GridRow; }
	int32 GetGridCol() const { return GridCol; }
	EBattleLayer GetGridLayer() const { return GridLayer; }

	void SetGridPosition(int32 Row, int32 Col, EBattleLayer Layer);

	bool IsOnFlank() const { return bIsOnFlank; }
	void SetOnFlank(bool bOnFlank) { bIsOnFlank = bOnFlank; }
	FRotator GetOriginalRotation() const { return OriginalRotation; }
	void SetOriginalRotation(const FRotator& Rotation) { OriginalRotation = Rotation; }

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	bool bIsOnFlank = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	FRotator OriginalRotation = FRotator::ZeroRotator;

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

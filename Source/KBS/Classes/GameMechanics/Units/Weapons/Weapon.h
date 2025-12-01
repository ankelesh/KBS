#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTypes/DamageTypes.h"
#include "Weapon.generated.h"

class UBattleEffect;
class UWeaponDataAsset;

USTRUCT(BlueprintType)
struct FAreaShape
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	TArray<FIntPoint> RelativeCells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	bool bAffectsAllLayers = false;
};

USTRUCT(BlueprintType)
struct FWeaponStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TMap<EDamageSource, int32> DamagePerSource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float AccuracyMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	ETargetReach TargetReach = ETargetReach::AnyEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	FAreaShape AreaShape;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KBS_API UWeapon : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeapon();

	void InitializeFromDataAsset();
	void RecalculateModifiedStats();
	void Restore();

	const FWeaponStats& GetStats() const { return ModifiedStats; }
	const TArray<TObjectPtr<UBattleEffect>>& GetEffects() const { return ActiveEffects; }
	const ETargetReach GetReach() const { return ModifiedStats.TargetReach; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UWeaponDataAsset> Config;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FWeaponStats BaseStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FWeaponStats ModifiedStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<TObjectPtr<UBattleEffect>> ActiveEffects;
};

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTypes/DamageTypes.h"
#include "Weapon.generated.h"

class UBattleEffect;
class UWeaponDataAsset;
class UUnitVisualsComponent;
class UStaticMeshComponent;

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
	int32 BaseDamage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSet<EDamageSource> DamageSources;

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

	void Initialize(UUnitVisualsComponent* VisualsComp, UWeaponDataAsset* Data);
	void InitializeFromDataAsset();
	void RecalculateModifiedStats();
	void Restore();

	const FWeaponStats& GetStats() const { return ModifiedStats; }
	const TArray<TObjectPtr<UBattleEffect>>& GetEffects() const { return ActiveEffects; }
	const ETargetReach GetReach() const { return ModifiedStats.TargetReach; }
	UStaticMeshComponent* GetWeaponMesh() const { return WeaponMeshComponent; }
	const TObjectPtr<UWeaponDataAsset> GetConfig() const { return Config; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UWeaponDataAsset> Config;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FWeaponStats BaseStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FWeaponStats ModifiedStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<TObjectPtr<UBattleEffect>> ActiveEffects;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMeshComponent;
};

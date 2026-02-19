#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameMechanics/Units/Stats/BaseUnitStatTypes.h"
#include "Weapon.generated.h"
class UBattleEffect;
class UWeaponDataAsset;
class UUnitVisualsComponent;
class AUnit;
USTRUCT(BlueprintType)
struct FAreaShape
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	TArray<FIntPoint> RelativeCells;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	EShapeLayering ShapeLayering = EShapeLayering::BothLayerArea;
};
USTRUCT(BlueprintType)
struct FWeaponStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FUnitStatPositive BaseDamage {10};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FDamageSourceSetStat DamageSources;

	// Constant weapon property
	// Not modified by buffs - all accuracy buffs affect Unit stats instead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	int32 AccuracyMultiplier = 100;

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

	// Add enchantments/buffs to weapon stats (e.g., Stats.BaseDamage.AddFlatModifier(...))
	FWeaponStats& GetMutableStats() { return Stats; }
	FWeaponStats& GetStats() { return Stats; }
	const FWeaponStats& GetStats() const { return Stats; }
	const TArray<UBattleEffect*>& GetEffects() const { return ActiveEffects; }
	FText GetEffectsTooltips(AUnit* Owner);
	const ETargetReach GetReach() const { return Stats.TargetReach; }
	const UWeaponDataAsset* GetConfig() const { return Config; }
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UWeaponDataAsset> Config;

	// Single stats instance - stat wrappers handle Base/Modified internally
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FWeaponStats Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<TObjectPtr<UBattleEffect>> ActiveEffects;
};

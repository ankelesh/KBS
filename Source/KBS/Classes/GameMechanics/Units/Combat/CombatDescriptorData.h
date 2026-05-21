#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/CombatDescriptorTypes.h"
#include "GameMechanics/Units/Stats/BaseUnitStatTypes.h"
#include "CombatDescriptorData.generated.h"

class UBattleEffect;
class UBattleEffectDataAsset;

USTRUCT(BlueprintType)
struct FCombatDescriptorStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FUnitStatPositive BaseMagnitude {10};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FDamageSourceSetStat DamageSources;

	// Constant descriptor property
	// Not modified by buffs - all accuracy buffs affect Unit stats instead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 AccuracyMultiplier = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	ETargetReach TargetReach = ETargetReach::AnyEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	FAreaShape AreaShape;
};

USTRUCT(BlueprintType)
struct FDescriptorEffectConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UBattleEffect> EffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UBattleEffectDataAsset> EffectConfig;
};

USTRUCT(BlueprintType)
struct FCombatDescriptorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor Stats")
	FCombatDescriptorStats BaseStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor Effects")
	TArray<FDescriptorEffectConfig> Effects;

	// None = sentinel (no inline override); asset always sets this explicitly
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor")
	EMagnitudePolicy MagnitudePolicy = EMagnitudePolicy::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bIsImmutable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bGuaranteedHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Side Effects")
	FDescriptorSideEffects SideEffects;
};

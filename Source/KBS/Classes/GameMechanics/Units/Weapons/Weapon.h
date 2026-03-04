#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/WeaponTypes.h"
#include "GameMechanics/Units/Stats/BaseUnitStatTypes.h"
#include "Weapon.generated.h"
class UBattleEffect;
class UWeaponDataAsset;
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
UCLASS(BlueprintType)
class KBS_API UWeapon : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(UObject* Outer, UWeaponDataAsset* Data, int32 BaseDamageOverride = -1);

	
	static EAttackIntent DeduceAttackIntent(const UWeapon * Weapon);
	// Add enchantments/buffs to weapon stats (e.g., Stats.BaseDamage.AddFlatModifier(...))
	void ModifyDamage(int32 Damage, FGuid ModificatorGuid, bool bIsFlat);
	void RemoveDamageModifier(int32 Damage, FGuid ModificatorGuid, bool bIsFlat);
	void ModifySource(const TSet<EDamageSource>& Sources, FGuid ModificatorGuid);
	void RemoveSourceModifier(FGuid ModificatorGuid);
	
	bool IsMutable() const;
	bool IsRequiringAccuracyRoll() const { return bGuaranteedHit;};
	EAttackIntent GetIntent() const;
	const FWeaponStats& GetStats() const { return Stats; }
	const TArray<UBattleEffect*>& GetEffects() const { return ActiveEffects; }
	FText GetEffectsTooltips(AUnit* Owner);
	ETargetReach GetReach() const { return Stats.TargetReach; }
	const UWeaponDataAsset* GetConfig() const { return Config; }
	EWeaponDesignation GetDesignation() const { return Designation; }
	bool IsUsableForAutoAttack() const { return Designation != EWeaponDesignation::Spells; }
	bool IsUsableForSpells() const { return Designation != EWeaponDesignation::AutoAttacks; }
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UWeaponDataAsset> Config;

	// Single stats instance - stat wrappers handle Base/Modified internally
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponDesignation Designation = EWeaponDesignation::AllPurpose;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	EAttackIntent Intent = EAttackIntent::Auto;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FWeaponStats Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	bool bIsImmutable;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	bool bGuaranteedHit;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<TObjectPtr<UBattleEffect>> ActiveEffects;
};


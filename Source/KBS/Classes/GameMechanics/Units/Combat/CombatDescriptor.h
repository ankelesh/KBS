#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/CombatDescriptorTypes.h"
#include "GameMechanics/Units/Stats/BaseUnitStatTypes.h"
#include "CombatDescriptor.generated.h"
class UBattleEffect;
class UCombatDescriptorDataAsset;
class AUnit;
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
UCLASS(BlueprintType)
class KBS_API UCombatDescriptor : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(UObject* Outer, UCombatDescriptorDataAsset* Data, int32 BaseMagnitudeOverride = -1);

	
	static ECombatIntent DeduceAttackIntent(const UCombatDescriptor* Descriptor);
	// Add enchantments/buffs to descriptor stats (e.g., Stats.BaseMagnitude.AddFlatModifier(...))
	void ModifyMagnitude(int32 Magnitude, FGuid ModificatorGuid, bool bIsFlat);
	void RemoveMagnitudeModifier(int32 Magnitude, FGuid ModificatorGuid, bool bIsFlat);
	void ModifySource(const TSet<EDamageSource>& Sources, FGuid ModificatorGuid);
	void RemoveSourceModifier(FGuid ModificatorGuid);
	
	bool IsMutable() const;
	void SetMagnitudeBase(int32 Magnitude);
	bool IsRequiringAccuracyRoll() const { return bGuaranteedHit;};
	ECombatIntent GetIntent() const;
	const FCombatDescriptorStats& GetStats() const { return Stats; }
	const TArray<UBattleEffect*>& GetEffects() const { return ActiveEffects; }
	FText GetEffectsTooltips(AUnit* Owner);
	ETargetReach GetReach() const { return Stats.TargetReach; }
	const UCombatDescriptorDataAsset* GetConfig() const { return Config; }
	ECombatDescriptorDesignation GetDesignation() const { return Designation; }
	bool IsUsableForAutoAttack() const { return Designation != ECombatDescriptorDesignation::Spells; }
	bool IsUsableForSpells() const { return Designation != ECombatDescriptorDesignation::AutoAttacks; }
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Descriptor")
	TObjectPtr<UCombatDescriptorDataAsset> Config;

	// Single stats instance - stat wrappers handle Base/Modified internally
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Descriptor")
	ECombatDescriptorDesignation Designation = ECombatDescriptorDesignation::AllPurpose;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	ECombatIntent Intent = ECombatIntent::Auto;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Descriptor")
	FCombatDescriptorStats Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	bool bIsImmutable;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	bool bGuaranteedHit;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Descriptor")
	TArray<TObjectPtr<UBattleEffect>> ActiveEffects;
};


#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "OffensiveSpellAbilityDefinition.generated.h"

class UAnimMontage;

UCLASS(BlueprintType)
class KBS_API UOffensiveSpellAbilityDefinition : public UUnitAbilityDefinition
{
	GENERATED_BODY()
public:
	// Inline weapon config — a transient UWeaponDataAsset is constructed from these at init time
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell|Weapon")
	FWeaponStats EmbeddedStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell|Weapon")
	TArray<FWeaponEffectConfig> EmbeddedEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell|Weapon")
	TObjectPtr<UAnimMontage> AttackMontage;

	// Scaling: embedded.BaseDamage = spellWeapon.ModifiedDamage * DamageMultiplier + FlatDamageBonus
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell|Scaling")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell|Scaling")
	int32 FlatDamageBonus = 0;

	// Skip scaling entirely; embedded weapon's base damage is used as-is
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell|Scaling")
	bool bIgnoreScaling = false;
};

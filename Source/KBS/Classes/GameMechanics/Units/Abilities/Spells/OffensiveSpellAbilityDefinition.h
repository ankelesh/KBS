#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Combat/CombatDescriptorDataAsset.h"
#include "OffensiveSpellAbilityDefinition.generated.h"

class UAnimMontage;

UCLASS(BlueprintType)
class KBS_API UOffensiveSpellAbilityDefinition : public UUnitAbilityDefinition
{
	GENERATED_BODY()
public:
	// Inline descriptor config — a transient UCombatDescriptorDataAsset is constructed from these at init time
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell|Descriptor")
	FCombatDescriptorStats EmbeddedStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell|Descriptor")
	TArray<FDescriptorEffectConfig> EmbeddedEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell|Descriptor")
	TObjectPtr<UAnimMontage> AttackMontage;

	// Scaling: embedded.BaseDamage = spellDescriptor.ModifiedDamage * DamageMultiplier + FlatDamageBonus
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell|Scaling")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell|Scaling")
	int32 FlatDamageBonus = 0;

	// Skip scaling entirely; embedded descriptor's base damage is used as-is
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell|Scaling")
	bool bIgnoreScaling = false;
};

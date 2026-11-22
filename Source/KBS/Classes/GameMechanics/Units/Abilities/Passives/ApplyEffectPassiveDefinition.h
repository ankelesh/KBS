#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Combat/CombatDescriptorData.h"
#include "ApplyEffectPassiveDefinition.generated.h"

/** Passive that applies its effects to the owner once, when the battle starts (or on spawn if it already started). */
UCLASS(BlueprintType)
class KBS_API UApplyEffectPassiveDefinition : public UUnitAbilityDefinition
{
	GENERATED_BODY()
public:
	UApplyEffectPassiveDefinition();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apply Effect")
	TArray<FDescriptorEffectConfig> Effects;
};

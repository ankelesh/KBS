#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Combat/CombatDescriptorData.h"
#include "CounterReactionPassiveDefinition.generated.h"

/**
 * Counter, Eternal Vengeance and Mummy's Curse are all this definition:
 *  - bUseWeapon only            : counter-attack with a reaching weapon, nothing otherwise.
 *  - bUseWeapon + own descriptor: weapon when one reaches the attacker, own descriptor otherwise.
 *  - own descriptor only        : always the own descriptor, weapons and their costs are never used.
 * Uses per turn come from MaxUses (0 = unlimited).
 */
UCLASS(BlueprintType)
class KBS_API UCounterReactionPassiveDefinition : public UUnitAbilityDefinition
{
	GENERATED_BODY()
public:
	UCounterReactionPassiveDefinition();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Counter")
	bool bUseWeapon = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Counter")
	bool bUseOwnDescriptor = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Counter", meta = (EditCondition = "bUseOwnDescriptor"))
	FCombatDescriptorData OwnDescriptor;

	// Presentation of the own-descriptor counter, same as a spell's AnimTag.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Counter", meta = (EditCondition = "bUseOwnDescriptor"))
	FGameplayTag OwnAnimTag;
};

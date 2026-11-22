#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Combat/CombatDescriptorData.h"
#include "GameMechanics/Units/Components/Config/UnitChargeCost.h"
#include "ChargedAutoAttackAbilityDefinition.generated.h"

UCLASS(BlueprintType)
class KBS_API UChargedAutoAttackAbilityDefinition : public UUnitAbilityDefinition
{
	GENERATED_BODY()
public:
	// Used instead of the weapon descriptor while the owner can afford ChargeCost; the weapon descriptor is the fallback.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChargedAttack")
	FCombatDescriptorData ChargedDescriptor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChargedAttack")
	FUnitChargeCost ChargeCost;
};

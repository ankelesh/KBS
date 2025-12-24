#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "UnitFleeAbility.generated.h"
UCLASS(Blueprintable)
class KBS_API UUnitFleeAbility : public UUnitAbilityInstance
{
	GENERATED_BODY()
public:
	virtual FAbilityResult ApplyAbilityEffect(const FAbilityBattleContext& Context) override;
};

#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "UnitMovementAbility.generated.h"

UCLASS(Blueprintable)
class KBS_API UUnitMovementAbility : public UUnitAbilityInstance
{
	GENERATED_BODY()
public:
	virtual ETargetReach GetTargeting() const override;
	virtual FAbilityResult ApplyAbilityEffect(const FAbilityBattleContext& Context) override;
	virtual FAbilityValidation CanExecute(const FAbilityBattleContext& Context) const override;
};

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
	virtual FAbilityResult ApplyAbilityEffect(AUnit* SourceUnit, FTacCoordinates TargetCell) override;
	virtual FAbilityValidation CanExecute(AUnit* SourceUnit, FTacCoordinates TargetCell) const override;
};

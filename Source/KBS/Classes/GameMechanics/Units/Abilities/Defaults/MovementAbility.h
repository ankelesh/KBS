#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "MovementAbility.generated.h"

UCLASS(Blueprintable)
class KBS_API UMovementAbility : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual FTargetingDescriptor GetTargeting() const override;
	virtual FGameplayTagContainer BuildTags() const override;
	virtual FAbilityExecutionResult Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;

};

#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "DefendAbility.generated.h"


UCLASS(Blueprintable)
class KBS_API UDefendAbility : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual FAbilityExecutionResult Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;
	virtual FGameplayTagContainer BuildTags() const override;
};


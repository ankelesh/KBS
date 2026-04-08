#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "FleeAbility.generated.h"

UCLASS(Blueprintable)
class KBS_API UFleeAbility : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual FAbilityExecutionResult Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;
	virtual FGameplayTagContainer BuildTags() const override;
	virtual void HandleTurnEnd() override {}

private:
	UFUNCTION()
	void HandleTurnStarted(AUnit* Unit);
	void OnFleeRotationCompleted();
};

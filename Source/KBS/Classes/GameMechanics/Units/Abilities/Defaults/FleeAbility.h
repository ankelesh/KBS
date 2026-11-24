#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "FleeAbility.generated.h"

class UFleeAbilityDefinition;

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

protected:
	// Arms the trigger that removes the owner from the field, according to the definition's Timing.
	virtual void ScheduleDeparture();
	// Places the owner off the field and logs the exit.
	void Depart();
	const UFleeAbilityDefinition* GetFleeConfig() const;

private:
	UFUNCTION()
	void HandleTurnStarted(AUnit* Unit);
	UFUNCTION()
	void HandleTurnEnded(AUnit* Unit);
};

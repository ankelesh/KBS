#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "ReloadAbility.generated.h"

class UReloadAbilityDefinition;

UCLASS(Blueprintable)
class KBS_API UReloadAbility : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual FAbilityExecutionResult Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;

private:
	const UReloadAbilityDefinition& GetReloadConfig() const;
};

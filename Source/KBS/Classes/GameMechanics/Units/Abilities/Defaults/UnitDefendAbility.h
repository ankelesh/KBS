#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "UnitDefendAbility.generated.h"


UCLASS(Blueprintable)
class KBS_API UUnitDefendAbility : public UUnitAbilityInstance
{
	GENERATED_BODY()
public:
	virtual bool Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;

};


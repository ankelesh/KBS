#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "UnitDefendAbility.generated.h"


UCLASS(Blueprintable)
class KBS_API UUnitDefendAbility : public UUnitAbilityInstance
{
	GENERATED_BODY()
public:
	virtual FAbilityResult ApplyAbilityEffect(AUnit* SourceUnit, FTacCoordinates TargetCell) override;
};


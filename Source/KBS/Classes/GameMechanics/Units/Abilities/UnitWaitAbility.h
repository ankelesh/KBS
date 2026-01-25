#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "UnitWaitAbility.generated.h"

UCLASS(Blueprintable)
class KBS_API UUnitWaitAbility : public UUnitAbilityInstance
{
	GENERATED_BODY()
public:
	UUnitWaitAbility();
	virtual FAbilityResult ApplyAbilityEffect(AUnit* SourceUnit, FTacCoordinates TargetCell) override;
};

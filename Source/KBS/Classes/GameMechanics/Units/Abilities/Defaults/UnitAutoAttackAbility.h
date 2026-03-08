#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "UnitAutoAttackAbility.generated.h"


UCLASS(Blueprintable)
class KBS_API UUnitAutoAttackAbility : public UUnitAbilityInstance
{
public:
	virtual TMap<FTacCoordinates, FPreviewHitResult> DamagePreview(FTacCoordinates TargetCell) const override;

	virtual FAbilityExecutionResult Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;
	

private:
	GENERATED_BODY()
	virtual EAbilityTurnReleasePolicy DecideTurnRelease() const override;
public:
	virtual ETargetReach GetTargeting() const override;
};


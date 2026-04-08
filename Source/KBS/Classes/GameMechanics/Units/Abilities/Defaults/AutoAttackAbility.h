#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "AutoAttackAbility.generated.h"


UCLASS(Blueprintable)
class KBS_API UAutoAttackAbility : public UUnitAbility
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
	virtual FTargetingDescriptor GetTargeting() const override;
	virtual FGameplayTagContainer BuildTags() const override;
};


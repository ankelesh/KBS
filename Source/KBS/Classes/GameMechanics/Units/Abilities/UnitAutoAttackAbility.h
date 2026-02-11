#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "UnitAutoAttackAbility.generated.h"


UCLASS(Blueprintable)
class KBS_API UUnitAutoAttackAbility : public UUnitAbilityInstance
{
public:
	virtual TMap<FTacCoordinates, FPreviewHitResult> DamagePreview(FTacCoordinates TargetCell) const override;

	virtual bool Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;

	virtual void Subscribe() override;
	virtual void Unsubscribe() override;

private:
	GENERATED_BODY()

	UFUNCTION()
	void HandleTurnEnd(AUnit*);

public:
	virtual ETargetReach GetTargeting() const override;
};


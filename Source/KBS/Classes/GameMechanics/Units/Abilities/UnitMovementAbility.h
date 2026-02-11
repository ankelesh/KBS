#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "UnitMovementAbility.generated.h"

UCLASS(Blueprintable)
class KBS_API UUnitMovementAbility : public UUnitAbilityInstance
{
	GENERATED_BODY()
public:
	virtual bool Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;

	virtual void Subscribe() override;
	virtual void Unsubscribe() override;

private:
	UFUNCTION()
	void HandleTurnEnd(AUnit* Self);
};

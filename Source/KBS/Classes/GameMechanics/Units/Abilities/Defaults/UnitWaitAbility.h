#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "UnitWaitAbility.generated.h"

UCLASS(Blueprintable)
class KBS_API UUnitWaitAbility : public UUnitAbilityInstance
{
	GENERATED_BODY()
public:
	virtual bool Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;

	virtual void HandleTurnEnd() override {} // charges restore on round end via Subscribe, not per-turn
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;

private:
	UFUNCTION()
	void HandleRoundEnd(int32 Round);
};

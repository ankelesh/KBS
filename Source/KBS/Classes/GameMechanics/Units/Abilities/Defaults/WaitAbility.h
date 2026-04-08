#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "WaitAbility.generated.h"

UCLASS(Blueprintable)
class KBS_API UWaitAbility : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual FAbilityExecutionResult Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;

	virtual FGameplayTagContainer BuildTags() const override;
	virtual void HandleTurnEnd() override {} // charges restore on round end via Subscribe, not per-turn
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;

private:
	UFUNCTION()
	void HandleRoundEnd(int32 Round);
};

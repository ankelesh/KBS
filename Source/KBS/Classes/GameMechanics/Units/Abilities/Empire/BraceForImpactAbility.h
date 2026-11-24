#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/Defaults/DefendAbility.h"
#include "BraceForImpactAbility.generated.h"

/**
 * Defend that also arms one counter-attack: while the owner is still Defending, the first damaging hit
 * from an attacker the owner's weapon can reach is answered with a reaction attack.
 * The counter is spent by firing and disarmed when the owner's next turn starts.
 */
UCLASS()
class KBS_API UBraceForImpactAbility : public UDefendAbility
{
	GENERATED_BODY()
public:
	virtual FAbilityExecutionResult Execute(FTacCoordinates TargetCell) override;
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;

private:
	UFUNCTION()
	void HandleOwnerDamaged(AUnit* Unit, AUnit* Attacker);
	UFUNCTION()
	void HandleOwnerTurnStart(AUnit* Unit);

	bool bCounterArmed = false;
};

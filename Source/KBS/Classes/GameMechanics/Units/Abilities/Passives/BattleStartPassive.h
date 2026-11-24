#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "BattleStartPassive.generated.h"

/**
 * Passive that runs OnBattleStart() exactly once: when the turn subsystem announces the battle start,
 * or right away if the owner joins (e.g. a summon) after it. Subclasses only implement the reaction.
 */
UCLASS(Abstract)
class KBS_API UBattleStartPassive : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;
	virtual bool CanExecute() const override { return false; }

protected:
	virtual void OnBattleStart() {}

private:
	UFUNCTION()
	void HandleBattleStart();

	bool bBattleStartHandled = false;
};

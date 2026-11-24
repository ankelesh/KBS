#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "PaveesaPassive.generated.h"

/**
 * Passive: damage the owner takes from attackers who are not adjacent (grid distance > 1) is halved.
 * Applies to any damaging attack, magic included. Halving is on the final result, after armour, damage reduction and Defend.
 *
 * Subscribe() binds to Owner->OnCombatResolutionResultApplied (target side, result not yet applied).
 */
UCLASS()
class KBS_API UPaveesaPassive : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;
	virtual bool CanExecute() const override { return false; }

private:
	void OnResultApplied(FCombatContext& Context, FHitInstance& Hit, FDamageResult& Result);
};

#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/Defaults/AutoAttackAbility.h"
#include "GameMechanics/Units/Combat/Weapon.h"
#include "CombinedWarfareAbility.generated.h"

/**
 * Auto-attack that may be used more than once per turn (MaxUses on the definition), but never twice with the same weapon.
 * The weapon of the previous attack is excluded from targeting, preview and execution until the turn ends.
 * Multi-use auto-attacks lock the turn to this ability after each use, so the follow-up attack is forced.
 */
UCLASS(Blueprintable)
class KBS_API UCombinedWarfareAbility : public UAutoAttackAbility
{
	GENERATED_BODY()
public:
	virtual void HandleTurnEnd() override;
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;

protected:
	virtual const UWeapon* GetExcludedWeapon() const override { return LastUsedWeapon.Get(); }
	virtual void OnWeaponUsed(const UWeapon& UsedWeapon) override { LastUsedWeapon = &UsedWeapon; }

private:
	// Turn start also clears the exclusion: a unit that died mid-turn never reached HandleTurnEnd
	UFUNCTION()
	void HandleTurnStarted(AUnit* Unit);

	TWeakObjectPtr<const UWeapon> LastUsedWeapon;
};

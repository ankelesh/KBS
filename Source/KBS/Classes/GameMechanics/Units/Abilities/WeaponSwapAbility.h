#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "WeaponSwapAbility.generated.h"

class UWeapon;

/**
 * Spellbook ability that replaces unit's weapons with a spell weapon,
 * then automatically equips the auto-attack ability.
 * Used for weapon-based spells like fireballs, ice bolts, buffs, etc.
 */
UCLASS(Blueprintable)
class KBS_API UWeaponSwapAbility : public UUnitAbilityInstance
{
	GENERATED_BODY()

public:
	virtual FAbilityResult ApplyAbilityEffect(AUnit* SourceUnit, FTacCoordinates TargetCell) override;
	virtual ETargetReach GetTargeting() const override;

protected:
	/** The weapon to replace current weapons with (e.g., Fireball, Ice Bolt, Healing Touch) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell Weapon")
	TSubclassOf<UWeapon> SpellWeapon;
};

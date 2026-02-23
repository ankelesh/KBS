#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/CombatTypes.h"

class AUnit;

class UBattleEffect;
enum class ETargetReach : uint8;

/**
 * Pure stateless damage calculation utilities.
 * All methods are static - no subsystem overhead.
 */
class KBS_API FDamageCalculation
{
public:
	// Damage calculations
	static float CalculateHitChance(AUnit* Attacker, UWeapon*Weapon, AUnit* Target);
	static FDamageResult CalculateDamage(AUnit* Attacker, UWeapon*Weapon, AUnit* Target);
	static float CalculateEffectApplication(AUnit* Attacker, UBattleEffect* Effect, AUnit* Target);

	// Weapon selection
	static UWeapon* SelectMaxReachWeapon(AUnit* Unit, bool bAutoAttackOnly = false);
	static UWeapon* SelectSpellWeapon(AUnit* Unit);

	// Spell scaling: sets embedded weapon's base damage from spell weapon's modified damage
	static void ApplySpellScaling(UWeapon* EmbeddedWeapon, UWeapon* SpellWeapon, float Multiplier, int32 FlatBonus);

	// Combat resolution
	static FPreviewHitResult PreviewDamage(AUnit* Attacker, UWeapon*Weapon, AUnit* Target);

	static bool PerformAccuracyRoll(float HitChance);
	static bool IsFriendlyReach(ETargetReach Reach);
	static EDamageSource SelectBestDamageSource(const TSet<EDamageSource>& DamageSources, AUnit* Target);
};

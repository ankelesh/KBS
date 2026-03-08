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
	static float CalculateHitChance(AUnit* Attacker, UCombatDescriptor* Descriptor, AUnit* Target);
	static FDamageResult CalculateDamage(AUnit* Attacker, UCombatDescriptor* Descriptor, AUnit* Target);
	static float CalculateEffectApplication(AUnit* Attacker, UBattleEffect* Effect, AUnit* Target);
	static FDamageResult CalculateHeal(AUnit* Attacker, UCombatDescriptor* Descriptor, AUnit* Target);

	// Descriptor selection
	static UCombatDescriptor* SelectMaxReachDescriptor(AUnit* Unit, bool bAutoAttackOnly = false);
	static UCombatDescriptor* SelectSpellDescriptor(AUnit* Unit);
	static UCombatDescriptor* SelectDescriptorForTarget(AUnit* Attacker, AUnit* Target, bool bAutoAttackOnly = false);

	// Spell scaling: sets embedded descriptor's base damage from spell descriptor's modified damage
	static void ApplySpellScaling(UCombatDescriptor* EmbeddedDescriptor, UCombatDescriptor* SpellDescriptor, float Multiplier, int32 FlatBonus);

	// Combat resolution
	static FPreviewHitResult PreviewDamage(AUnit* Attacker, UCombatDescriptor* Descriptor, AUnit* Target);

	static bool PerformAccuracyRoll(float HitChance);
	static bool IsFriendlyReach(ETargetReach Reach);
	static EDamageSource SelectBestDamageSource(const TSet<EDamageSource>& DamageSources, AUnit* Target);
};

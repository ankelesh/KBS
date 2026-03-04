#pragma once
#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponDesignation : uint8
{
	AllPurpose  UMETA(DisplayName = "All Purpose"),
	AutoAttacks UMETA(DisplayName = "Auto Attacks Only"),
	Spells      UMETA(DisplayName = "Spells Only"),
};

UENUM(BlueprintType)
enum class EAttackIntent : uint8
{
	// implicitly guess from damage, targeting, source and effect list
	Auto     UMETA(DisplayName = "Auto deduction"),
	// deduced as damage < 0 && DamageSource = life
	// represents always-hitting no-roll heal amount
	Heal     UMETA(DisplayName = "Heal action"),
	// deduced as damage > 0 && DamageSource != life
	// represents normal flow
	Attack UMETA(DisplayName= "Normal Attack action"),
	// deduced as damage == 0 && effect.num() > 0
	// represents usual flow without damage calculations and applications phases
	EffectApplication UMETA(DisplayName = "Effect Application"),
	// deduced as damage == 0 && effect.num() > 0 && damage == immutable
	// represents always-hitting no-roll damage-phase-less effect application
	BuffApplication UMETA(DisplayName= "Buff Application"),
	// copy of EffectApplication to be better expressed
	DebuffApplication UMETA(DisplayName= "Debuff Application"),
	
};

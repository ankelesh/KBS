// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/CombatTypes.h"

void UUnitAbilityInstance::InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner)
{
	Config = InDefinition;
	Owner = InOwner;
	if (Config)
	{
		RemainingCharges = Config->MaxCharges;
	}
}

bool UUnitAbilityInstance::TriggerAbility(const FAbilityBattleContext& Context)
{
	return true;
}

void UUnitAbilityInstance::ApplyAbilityEffect(const FAbilityBattleContext& Context)
{
}

ETargetReach UUnitAbilityInstance::GetTargeting() const
{
	if (Config)
	{
		return Config->Targeting;
	}
	return ETargetReach::None;
}

bool UUnitAbilityInstance::IsPassive() const
{
	if (Config)
	{
		return Config->bIsPassive;
	}
	return false;
}

TMap<AUnit*, FPreviewHitResult> UUnitAbilityInstance::DamagePreview(AUnit* Source, const TArray<AUnit*>& Targets)
{
	return TMap<AUnit*, FPreviewHitResult>();
}

void UUnitAbilityInstance::Subscribe()
{
	if (!Owner)
	{
		return;
	}

	Owner->OnUnitAttacked.AddDynamic(this, &UUnitAbilityInstance::HandleUnitAttacked);
	Owner->OnUnitDamaged.AddDynamic(this, &UUnitAbilityInstance::HandleUnitDamaged);
	Owner->OnUnitAttacks.AddDynamic(this, &UUnitAbilityInstance::HandleUnitAttacks);
}

void UUnitAbilityInstance::Unsubscribe()
{
	if (!Owner)
	{
		return;
	}

	Owner->OnUnitAttacked.RemoveDynamic(this, &UUnitAbilityInstance::HandleUnitAttacked);
	Owner->OnUnitDamaged.RemoveDynamic(this, &UUnitAbilityInstance::HandleUnitDamaged);
	Owner->OnUnitAttacks.RemoveDynamic(this, &UUnitAbilityInstance::HandleUnitAttacks);
}

void UUnitAbilityInstance::HandleUnitAttacked(AUnit* Victim, AUnit* Attacker)
{
	// Base implementation does nothing; override in subclasses
}

void UUnitAbilityInstance::HandleUnitDamaged(AUnit* Victim, AUnit* Attacker)
{
	// Base implementation does nothing; override in subclasses
}

void UUnitAbilityInstance::HandleUnitAttacks(AUnit* Attacker, AUnit* Target)
{
	// Base implementation does nothing; override in subclasses
}

void UUnitAbilityInstance::ConsumeCharge()
{
	if (RemainingCharges > 0)
	{
		RemainingCharges--;
	}
}

void UUnitAbilityInstance::RestoreCharges()
{
	if (Config)
	{
		RemainingCharges = Config->MaxCharges;
	}
}

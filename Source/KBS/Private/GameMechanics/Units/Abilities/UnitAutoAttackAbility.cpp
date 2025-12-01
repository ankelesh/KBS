// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMechanics/Units/Abilities/UnitAutoAttackAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/DamageCalculator.h"
#include "GameplayTypes/CombatTypes.h"

ETargetReach UUnitAutoAttackAbility::GetTargeting() const
{
	if (Config->Targeting != ETargetReach::None)
	{
		return Config->Targeting;
	}
	else
	{
		UWorld* World = Owner->GetWorld();
		if (World)
		{
			UDamageCalculator* DamageCalc = World->GetSubsystem<UDamageCalculator>();
			if (DamageCalc)
			{
				UWeapon* Weapon = DamageCalc->SelectMaxReachWeapon(Owner);
				if (Weapon)
				{
					return Weapon->GetReach();
				}
			}
		}
	}
	return ETargetReach::None;
}

TMap<AUnit*, FPreviewHitResult> UUnitAutoAttackAbility::DamagePreview(AUnit* Source, const TArray<AUnit*>& Targets)
{
	TMap<AUnit*, FPreviewHitResult> Results;

	if (!Source)
	{
		return Results;
	}

	UWorld* World = Source->GetWorld();
	if (!World)
	{
		return Results;
	}

	UDamageCalculator* DamageCalc = World->GetSubsystem<UDamageCalculator>();
	if (!DamageCalc)
	{
		return Results;
	}

	ETargetReach Reach = GetTargeting();

	for (AUnit* Target : Targets)
	{
		if (Target)
		{
			FPreviewHitResult Preview = DamageCalc->PreviewDamage(Source, Target, Reach);
			Results.Add(Target, Preview);
		}
	}

	return Results;
}

void UUnitAutoAttackAbility::ApplyAbilityEffect(const FAbilityBattleContext& Context)
{
	if (!Context.SourceUnit)
	{
		return;
	}

	UWorld* World = Context.SourceUnit->GetWorld();
	if (!World)
	{
		return;
	}

	UDamageCalculator* DamageCalc = World->GetSubsystem<UDamageCalculator>();
	if (!DamageCalc)
	{
		return;
	}

	ETargetReach Reach = GetTargeting();

	for (AUnit* Target : Context.TargetUnits)
	{
		if (Target)
		{
			// Broadcast attack events
			Context.SourceUnit->OnUnitAttacks.Broadcast(Context.SourceUnit, Target);
			Target->OnUnitAttacked.Broadcast(Target, Context.SourceUnit);

			FCombatHitResult HitResult = DamageCalc->ProcessHit(Context.SourceUnit, Target, Reach);

			if (HitResult.bHit)
			{
				FDamageResult DamageResult;
				DamageResult.Damage = HitResult.Damage;
				DamageResult.DamageBlocked = HitResult.DamageBlocked;
				DamageResult.DamageSource = HitResult.DamageSource;

				Target->TakeHit(DamageResult);

				// Broadcast damage event
				Target->OnUnitDamaged.Broadcast(Target, Context.SourceUnit);
			}
		}
	}
}

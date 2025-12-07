// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMechanics/Units/Abilities/UnitAutoAttackAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Tactical/DamageCalculator.h"
#include "GameplayTypes/CombatTypes.h"
#include "Kismet/KismetMathLibrary.h"

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

FAbilityResult UUnitAutoAttackAbility::ApplyAbilityEffect(const FAbilityBattleContext& Context)
{
	if (!Context.SourceUnit)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom, FText::FromString("No source unit"));
	}

	UWorld* World = Context.SourceUnit->GetWorld();
	if (!World)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom, FText::FromString("No world"));
	}

	UDamageCalculator* DamageCalc = World->GetSubsystem<UDamageCalculator>();
	if (!DamageCalc)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom, FText::FromString("No damage calculator"));
	}

	ETargetReach Reach = GetTargeting();

	// Play attack animation
	UWeapon* Weapon = DamageCalc->SelectMaxReachWeapon(Context.SourceUnit);
	if (Weapon && Context.TargetUnits.Num() > 0)
	{
		UWeaponDataAsset* WeaponConfig = Weapon->GetConfig();
		UAnimMontage* AttackMontage = WeaponConfig ? WeaponConfig->AttackMontage : nullptr;

		// Rotate toward first target
		AUnit* FirstTarget = Context.TargetUnits[0];
		if (FirstTarget)
		{
			const FVector SourceLoc = Context.SourceUnit->GetActorLocation();
			const FVector TargetLoc = FirstTarget->GetActorLocation();
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(SourceLoc, TargetLoc);
			LookAtRotation.Yaw -= 90.0f; // Adjust for Y-oriented models

			if (Context.SourceUnit->VisualsComponent)
			{
				Context.SourceUnit->VisualsComponent->RotateTowardTarget(LookAtRotation, 540.0f);

				if (AttackMontage)
				{
					Context.SourceUnit->VisualsComponent->PlayAttackMontage(AttackMontage);
				}
			}
		}
	}

	// Create success result and populate affected units
	FAbilityResult Result = CreateSuccessResult();

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

				// Add to affected units
				Result.UnitsAffected.Add(Target);

				UE_LOG(LogTemp, Log, TEXT("  -> Target '%s' took %.1d damage (%.1d blocked)"),
					*Target->GetName(), HitResult.Damage, HitResult.DamageBlocked);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("  -> Target '%s' avoided the attack"),
					*Target->GetName());
			}
		}
	}

	return Result;
}

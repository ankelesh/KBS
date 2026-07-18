#pragma once
#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "GameplayTagContainer.h"
#include "GameplayTypes/CombatTypes.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/TeamConstants.h"
#include "GameplayTypes/LogTypesLibrary.h"
#include "GameMechanics/Units/Stats/UnitStatDelta.h"
#include "GameMechanics/Units/Stats/UnitStatusContainer.h"
#include "TacLogAbilitySteps.generated.h"

USTRUCT()
struct KBS_API FTacLogStepBase
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag Reason;
};

USTRUCT()
struct KBS_API FTacLogHitRecord
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid TargetId;

	UPROPERTY()
	FTacCoordinates TargetCoords;

	UPROPERTY()
	EHitOutcome Outcome = EHitOutcome::Miss;

	UPROPERTY()
	FDamageResult DamageResult;

	UPROPERTY()
	TArray<FAppliedEffectRef> AppliedEffects;

	UPROPERTY()
	int32 RemainingHp = 0;

	UPROPERTY()
	bool bKilledTarget = false;

	// Mirrors FCombatHitResult::HitChance. -1 = no roll required.
	UPROPERTY()
	float HitChance = -1.0f;
	// Actual roll value (0-100). Meaningful only when HitChance >= 0.
	UPROPERTY()
	float AccuracyRoll = -1.0f;
};

USTRUCT()
struct KBS_API FTacLogWaitStep : public FTacLogStepBase
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid UnitId;

	static FTacLogWaitStep Make(FGuid InUnitId)
	{
		FTacLogWaitStep Step;
		Step.UnitId = InUnitId;
		return Step;
	}
};

USTRUCT()
struct KBS_API FTacLogFleeStep : public FTacLogStepBase
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid UnitId;

	UPROPERTY()
	FTacCoordinates UnitCoords;

	UPROPERTY()
	ETeamSide TeamSide = ETeamSide::Attacker;

	static FTacLogFleeStep Make(FGuid InUnitId, FTacCoordinates InCoords, ETeamSide InTeamSide)
	{
		FTacLogFleeStep Step;
		Step.UnitId = InUnitId;
		Step.UnitCoords = InCoords;
		Step.TeamSide = InTeamSide;
		return Step;
	}
};

USTRUCT()
struct KBS_API FTacLogMoveStep : public FTacLogStepBase
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid UnitId;

	UPROPERTY()
	FTacCoordinates FromCoords;

	UPROPERTY()
	FTacCoordinates ToCoords;

	UPROPERTY()
	bool bIsAnimated = false;

	static FTacLogMoveStep Make(FGuid InUnitId, FTacCoordinates From, FTacCoordinates To, bool bInIsAnimated)
	{
		FTacLogMoveStep Step;
		Step.UnitId = InUnitId;
		Step.FromCoords = From;
		Step.ToCoords = To;
		Step.bIsAnimated = bInIsAnimated;
		return Step;
	}
};

USTRUCT()
struct KBS_API FTacLogStatAltStep : public FTacLogStepBase
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid SourceUnitId;

	UPROPERTY()
	FGuid TargetUnitId;

	UPROPERTY()
	FUnitStatDelta AppliedDelta;

	UPROPERTY()
	EStatModifierRemovalPolicy RemovalPolicy = EStatModifierRemovalPolicy::Permanent;

	static FTacLogStatAltStep Make(FGuid InSourceUnitId, FGuid InTargetUnitId, const FUnitStatDelta& InDelta,
	                               EStatModifierRemovalPolicy InPolicy = EStatModifierRemovalPolicy::Permanent)
	{
		FTacLogStatAltStep Step;
		Step.SourceUnitId  = InSourceUnitId;
		Step.TargetUnitId  = InTargetUnitId;
		Step.AppliedDelta  = InDelta;
		Step.RemovalPolicy = InPolicy;
		return Step;
	}
};

USTRUCT()
struct KBS_API FTacLogEffectSpawnStep : public FTacLogStepBase
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid TargetUnitId;

	UPROPERTY()
	FAppliedEffectRef EffectRef;

	static FTacLogEffectSpawnStep Make(FGuid InTargetUnitId, const FAppliedEffectRef& InRef)
	{
		FTacLogEffectSpawnStep Step;
		Step.TargetUnitId = InTargetUnitId;
		Step.EffectRef = InRef;
		return Step;
	}

	// Appends one step per applied effect across all hit results (skips non-applied outcomes).
	static void AppendFromHits(TArray<TInstancedStruct<FTacLogStepBase>>& OutSteps, const TArray<FCombatHitResult>& HitResults);
};

USTRUCT()
struct KBS_API FTacLogStatusChangeStep : public FTacLogStepBase
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid UnitId;

	UPROPERTY()
	EUnitStatus Status = EUnitStatus::Defending;

	UPROPERTY()
	bool bActivated = true;

	// Valid only for ref-counted statuses (TurnBlocked, Pinned, Silenced, Disoriented).
	// Invalid guid for bool-flag statuses (Defending, Fleeing, Channeling, Dead).
	UPROPERTY()
	FGuid ModifierId;

	static FTacLogStatusChangeStep Make(FGuid InUnitId, EUnitStatus InStatus, bool bInActivated,
	                                    FGuid InModifierId = FGuid())
	{
		FTacLogStatusChangeStep Step;
		Step.UnitId      = InUnitId;
		Step.Status      = InStatus;
		Step.bActivated  = bInActivated;
		Step.ModifierId  = InModifierId;
		return Step;
	}

	// Appends one Defending-cleared step per hit that removed the target's defensive stance.
	static void AppendDefendingClearedFromHits(TArray<TInstancedStruct<FTacLogStepBase>>& OutSteps, const TArray<FCombatHitResult>& HitResults);
};

USTRUCT()
struct KBS_API FTacLogCombatStep : public FTacLogStepBase
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid AttackerId;

	UPROPERTY()
	FTacCoordinates AttackerCoords;

	UPROPERTY()
	FGuid PrimaryTargetId;

	UPROPERTY()
	TArray<FTacLogHitRecord> HitRecords;

	// Snapshot of the weapon's swing animation tag at simulation time - not re-derived at replay
	// time, since the attacker's loadout/position may have changed by then.
	UPROPERTY()
	FGameplayTag WeaponAnimTag;

	static FTacLogCombatStep Make(FGuid AttackerId, FTacCoordinates AttackerCoords, FGuid PrimaryTargetId, const TArray<FCombatHitResult>& HitResults, FGameplayTag WeaponAnimTag);
};

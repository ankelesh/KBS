#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTypes/CombatTypes.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/TeamConstants.h"
#include "GameMechanics/Units/Stats/UnitStatDelta.h"
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

	static FTacLogStatAltStep Make(FGuid InSourceUnitId, FGuid InTargetUnitId, const FUnitStatDelta& InDelta)
	{
		FTacLogStatAltStep Step;
		Step.SourceUnitId = InSourceUnitId;
		Step.TargetUnitId = InTargetUnitId;
		Step.AppliedDelta = InDelta;
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

	static FTacLogCombatStep Make(FGuid AttackerId, FTacCoordinates AttackerCoords, FGuid PrimaryTargetId, const TArray<FCombatHitResult>& HitResults);
};

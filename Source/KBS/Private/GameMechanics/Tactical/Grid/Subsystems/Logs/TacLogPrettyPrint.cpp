// File: Source/KBS/Private/GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPrettyPrint.cpp
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPrettyPrint.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Units/Stats/UnitStatDelta.h"

// ── Step formatters ───────────────────────────────────────────────────────────

static FString FormatStep(const FTacLogWaitStep& S, FTacLogFormatCtx& Ctx)
{
	return FString::Printf(TEXT("- Wait    %s"), *Ctx.Unit(S.UnitId));
}

static FString FormatStep(const FTacLogFleeStep& S, FTacLogFormatCtx& Ctx)
{
	return FString::Printf(TEXT("- Flee    %s@(%d,%d) [%s]"),
		*Ctx.Unit(S.UnitId), S.UnitCoords.Row, S.UnitCoords.Col,
		*ShortTeam(S.TeamSide));
}

static FString FormatStep(const FTacLogMoveStep& S, FTacLogFormatCtx& Ctx)
{
	return FString::Printf(TEXT("- Move    %s  (%d,%d)→(%d,%d)"),
		*Ctx.Unit(S.UnitId),
		S.FromCoords.Row, S.FromCoords.Col,
		S.ToCoords.Row,   S.ToCoords.Col);
}

static FString FormatStep(const FTacLogStatAltStep& S, FTacLogFormatCtx& Ctx)
{
	const FUnitStatDelta& D = S.AppliedDelta;
	FString Deltas;
	if (D.MaxHealth != 0)           Deltas += FString::Printf(TEXT(" HP%+d"), D.MaxHealth);
	if (D.Initiative != 0)          Deltas += FString::Printf(TEXT(" Init%+d"), D.Initiative);
	if (D.Accuracy != 0)            Deltas += FString::Printf(TEXT(" Acc%+d"), D.Accuracy);
	if (D.MagnitudeFlat != 0)       Deltas += FString::Printf(TEXT(" MagFlat%+d"), D.MagnitudeFlat);
	if (D.MagnitudeMultiplier != 0) Deltas += FString::Printf(TEXT(" MagMul%+d"), D.MagnitudeMultiplier);
	if (Deltas.IsEmpty()) Deltas = TEXT(" (no deltas)");

	return FString::Printf(TEXT("- StatAlt %s→%s |%s | %s"),
		*Ctx.Unit(S.SourceUnitId), *Ctx.Unit(S.TargetUnitId),
		*Deltas, *ShortPolicy(S.RemovalPolicy));
}

static FString FormatHitRecord(const FTacLogHitRecord& H, FTacLogFormatCtx& Ctx)
{
	FString ChanceStr = (H.HitChance >= 0.0f)
		? FString::Printf(TEXT(" [%d/%d%%]"), FMath::RoundToInt(H.AccuracyRoll), FMath::RoundToInt(H.HitChance))
		: TEXT(" [G/G]");

	FString Line = FString::Printf(TEXT("    %-6s %s%s"),
		*ShortOutcome(H.Outcome), *Ctx.Unit(H.TargetId), *ChanceStr);

	if (H.Outcome == EHitOutcome::Hit)
	{
		Line += FString::Printf(TEXT(": %d dmg"), H.DamageResult.Damage);
		if (H.DamageResult.DamageBlocked > 0)
			Line += FString::Printf(TEXT(" (%d blk)"), H.DamageResult.DamageBlocked);
		Line += FString::Printf(TEXT(" | →HP%d"), H.RemainingHp);
		if (H.bKilledTarget) Line += TEXT(" [DEAD]");

		for (const FAppliedEffectRef& Ref : H.AppliedEffects)
		{
			FString RollStr = (Ref.AppChance >= 0.0f)
				? FString::Printf(TEXT("[%d/%d%%]"), FMath::RoundToInt(Ref.AppRoll), FMath::RoundToInt(Ref.AppChance))
				: TEXT("[G/G]");

			if (Ref.WasApplied())
				Line += FString::Printf(TEXT(" +%s%s"), *AssetShortName(Ref.AssetId), *RollStr);
			else if (Ref.Outcome == EEffectApplicationOutcome::RollMissed)
				Line += FString::Printf(TEXT(" ~%s%s"), *AssetShortName(Ref.AssetId), *RollStr);
		}
	}
	return Line;
}

static FString FormatStep(const FTacLogCombatStep& S, FTacLogFormatCtx& Ctx)
{
	FString Result = FString::Printf(TEXT("- Combat  %s@(%d,%d) → %s"),
		*Ctx.Unit(S.AttackerId),
		S.AttackerCoords.Row, S.AttackerCoords.Col,
		*Ctx.Unit(S.PrimaryTargetId));

	for (const FTacLogHitRecord& H : S.HitRecords)
		Result += TEXT("\n") + FormatHitRecord(H, Ctx);

	return Result;
}

static FString FormatStep(const FTacLogEffectSpawnStep& S, FTacLogFormatCtx& Ctx)
{
	return FString::Printf(TEXT("- EffSpawn +%s → %s"),
		*AssetShortName(S.EffectRef.AssetId), *Ctx.Unit(S.TargetUnitId));
}

static FString FormatStep(const FTacLogStatusChangeStep& S, FTacLogFormatCtx& Ctx)
{
	FString StatusName = UEnum::GetValueAsString(S.Status);
	int32 ColonIdx = INDEX_NONE;
	if (StatusName.FindLastChar(TEXT(':'), ColonIdx))
		StatusName = StatusName.Mid(ColonIdx + 1);

	FString ModStr;
	if (S.ModifierId.IsValid())
		ModStr = FString::Printf(TEXT(" [mod:%s]"), *S.ModifierId.ToString().Left(8));

	return FString::Printf(TEXT("- Status  %s %s%s%s"),
		*Ctx.Unit(S.UnitId),
		S.bActivated ? TEXT("+") : TEXT("-"),
		*StatusName,
		*ModStr);
}

static FString DispatchFormatStep(const TInstancedStruct<FTacLogStepBase>& Step, FTacLogFormatCtx& Ctx)
{
	if (const FTacLogWaitStep*         S = Step.GetPtr<FTacLogWaitStep>())         return FormatStep(*S, Ctx);
	if (const FTacLogFleeStep*         S = Step.GetPtr<FTacLogFleeStep>())         return FormatStep(*S, Ctx);
	if (const FTacLogMoveStep*         S = Step.GetPtr<FTacLogMoveStep>())         return FormatStep(*S, Ctx);
	if (const FTacLogStatAltStep*      S = Step.GetPtr<FTacLogStatAltStep>())      return FormatStep(*S, Ctx);
	if (const FTacLogStatusChangeStep* S = Step.GetPtr<FTacLogStatusChangeStep>()) return FormatStep(*S, Ctx);
	if (const FTacLogCombatStep*       S = Step.GetPtr<FTacLogCombatStep>())       return FormatStep(*S, Ctx);
	if (const FTacLogEffectSpawnStep*  S = Step.GetPtr<FTacLogEffectSpawnStep>())  return FormatStep(*S, Ctx);
	return TEXT("- [UnknownStep]");
}

// ── Payload formatters ────────────────────────────────────────────────────────

static FString FormatPayload(const FAbilityUsePayload& P, FTacLogFormatCtx& Ctx)
{
	FString Result = FString::Printf(TEXT("→ (%d,%d) | %s"),
		P.Command.Row, P.Command.Col, *Ctx.Abil(P.AbilityInstanceId, P.AbilityAssetId));

	for (const TInstancedStruct<FTacLogStepBase>& Step : P.Steps)
		Result += TEXT("\n") + DispatchFormatStep(Step, Ctx);

	return Result;
}

static FString FormatPayload(const FTacEffectPayload& P, FTacLogFormatCtx& Ctx)
{
	FString Result = FString::Printf(TEXT("%s on %s  src:%s"),
		*Ctx.Fx(P.EffectInstanceId, P.EffectAssetId),
		*Ctx.Unit(P.OwnerUnitId),
		*Ctx.Unit(P.SourceId));

	for (const TInstancedStruct<FTacLogStepBase>& Step : P.Steps)
		Result += TEXT("\n") + DispatchFormatStep(Step, Ctx);

	return Result;
}

static FString FormatPayload(const FEffectEndPayload& P, FTacLogFormatCtx& Ctx)
{
	FString Result = FString::Printf(TEXT("%s on %s → %s"),
		*Ctx.Fx(P.EffectInstanceId, P.EffectAssetId),
		*Ctx.Unit(P.OwnerUnitId),
		*ShortRemoval(P.RemovalReason));

	for (const TInstancedStruct<FTacLogStepBase>& Step : P.Steps)
		Result += TEXT("\n") + DispatchFormatStep(Step, Ctx);

	return Result;
}

static FString FormatPayload(const FUnitMoveOffFieldPayload& P, FTacLogFormatCtx& Ctx)
{
	return FString::Printf(TEXT("%s@(%d,%d) [%s] | %s"),
		*Ctx.Unit(P.UnitId),
		P.LastFieldCoords.Row, P.LastFieldCoords.Col,
		*ShortTeam(P.TeamSide),
		P.bFled ? TEXT("fled") : TEXT("removed"));
}

static FString FormatPayload(const FTurnChangePayload& P, FTacLogFormatCtx& Ctx)
{
	FString From = P.OldTurnOwner.IsValid() ? Ctx.Unit(P.OldTurnOwner) : TEXT("-");
	FString To   = P.NewTurnOwner.IsValid() ? Ctx.Unit(P.NewTurnOwner) : TEXT("-");
	FString Kind = (P.ChangeKind == ETurnChangeKind::Round) ? TEXT("Round") : TEXT("Turn");
	return FString::Printf(TEXT("%s → %s [%s] | %s"),
		*From, *To, *ShortTeam(P.NewTurnTeam), *Kind);
}

static FString FormatPayload(const FUnitSpawnPayload& P, FTacLogFormatCtx& Ctx)
{
	FString Origin = P.bIsSummon
		? (TEXT("by ") + Ctx.Unit(P.SummonerUnitId))
		: TEXT("placed");
	return FString::Printf(TEXT("%s [%s]@(%d,%d) [%s] | %s"),
		*Ctx.Unit(P.SpawnedUnitId),
		*AssetShortName(P.UnitDefinitionId),
		P.SpawnCoords.Row, P.SpawnCoords.Col,
		*ShortTeam(P.TeamSide),
		*Origin);
}

static FString FormatPayload(const FUnitDespawnPayload& P, FTacLogFormatCtx& Ctx)
{
	return FString::Printf(TEXT("%s [%s] | %s"),
		*Ctx.Unit(P.UnitId),
		*ShortTeam(P.TeamSide),
		*ShortDespawnReason(P.Reason));
}

// ── Public dispatch ───────────────────────────────────────────────────────────

FString DispatchFormatPayload(const TInstancedStruct<FTacLogPayload>& Payload, FTacLogFormatCtx& Ctx)
{
	if (!Payload.IsValid()) return TEXT("<no payload>");
	if (const FAbilityUsePayload*       P = Payload.GetPtr<FAbilityUsePayload>())       return FormatPayload(*P, Ctx);
	if (const FTacEffectPayload*        P = Payload.GetPtr<FTacEffectPayload>())        return FormatPayload(*P, Ctx);
	if (const FEffectEndPayload*        P = Payload.GetPtr<FEffectEndPayload>())        return FormatPayload(*P, Ctx);
	if (const FUnitMoveOffFieldPayload* P = Payload.GetPtr<FUnitMoveOffFieldPayload>()) return FormatPayload(*P, Ctx);
	if (const FTurnChangePayload*       P = Payload.GetPtr<FTurnChangePayload>())       return FormatPayload(*P, Ctx);
	if (const FUnitSpawnPayload*        P = Payload.GetPtr<FUnitSpawnPayload>())        return FormatPayload(*P, Ctx);
	if (const FUnitDespawnPayload*      P = Payload.GetPtr<FUnitDespawnPayload>())      return FormatPayload(*P, Ctx);
	return TEXT("<no payload>");
}

// ── Legend ────────────────────────────────────────────────────────────────────

FString BuildTacLogLegend()
{
	return
		TEXT("=== Legend ===\n")
		TEXT("  State    C=Closed  !=Open(error)\n")
		TEXT("  Type     Ability | EffOn=EffectActivation | EffOff=EffectEnd | Turn=TurnChange | Exit=UnitExitField | Spawn=UnitSpawn | Despawn=UnitDespawn\n")
		TEXT("  Origin   Init=Initiated  React=Reaction  Trig=Triggered  Prog=Progressed\n")
		TEXT("  Team     Atk=Attacker  Def=Defender\n")
		TEXT("  Hit      HIT  MISS  IMMUNE  WARDED  CANCEL\n")
		TEXT("  Policy   perm=Permanent  insta=InstaRemove  dur=DurationControlled  own=OwnerControlled\n")
		TEXT("  Removal  Expired  Dispelled  Replaced  OwnerDied\n")
		TEXT("  |#N      event sequence position in spine\n")
		TEXT("  Steps    Move  Combat  EffSpawn(+effect)  StatAlt(stat delta)  Status(+set/-clear)  Wait  Flee\n")
		TEXT("  Units    TypeName-name  e.g. Warrior-alpha, Archer-beta (same type shares prefix)\n")
		TEXT("  !! Unit death is marked [DEAD] in ALL CAPS on the hit line — search for [DEAD] to find kills !!\n")
		TEXT("==============\n\n");
}

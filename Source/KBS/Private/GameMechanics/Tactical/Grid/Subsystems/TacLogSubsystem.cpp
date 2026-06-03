// File: Source/KBS/Private/GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.cpp
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Units/Stats/UnitStatDelta.h"
#include "GameplayTypes/CombatTypes.h"
#include "GameplayTypes/GridCoordinates.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"

DEFINE_LOG_CATEGORY_STATIC(LogTacLog, Log, All);

// ── Step formatters ──────────────────────────────────────────────────────────

static FString FormatStep(const FTacLogWaitStep& S)
{
	return FString::Printf(TEXT("[Wait] Unit=%s"), *S.UnitId.ToString());
}

static FString FormatStep(const FTacLogFleeStep& S)
{
	return FString::Printf(TEXT("[Flee] Unit=%s @ (%d,%d) Team=%s"),
		*S.UnitId.ToString(), S.UnitCoords.X, S.UnitCoords.Y,
		*UEnum::GetValueAsString(S.TeamSide));
}

static FString FormatStep(const FTacLogMoveStep& S)
{
	return FString::Printf(TEXT("[Move] Unit=%s (%d,%d)->(%d,%d) animated=%s"),
		*S.UnitId.ToString(),
		S.FromCoords.X, S.FromCoords.Y,
		S.ToCoords.X, S.ToCoords.Y,
		S.bIsAnimated ? TEXT("true") : TEXT("false"));
}

static FString FormatStep(const FTacLogStatAltStep& S)
{
	FString Header = FString::Printf(TEXT("[StatAlt] %s->%s"),
		*S.SourceUnitId.ToString(), *S.TargetUnitId.ToString());

	const FUnitStatDelta& D = S.AppliedDelta;
	FString Numerics;
	if (D.MaxHealth != 0)        Numerics += FString::Printf(TEXT(" HP=%d"), D.MaxHealth);
	if (D.Initiative != 0)       Numerics += FString::Printf(TEXT(" Init=%d"), D.Initiative);
	if (D.Accuracy != 0)         Numerics += FString::Printf(TEXT(" Acc=%d"), D.Accuracy);
	if (D.MagnitudeFlat != 0)    Numerics += FString::Printf(TEXT(" MagFlat=%d"), D.MagnitudeFlat);
	if (D.MagnitudeMultiplier != 0) Numerics += FString::Printf(TEXT(" MagMul=%d"), D.MagnitudeMultiplier);

	FString Detail = Numerics.IsEmpty() ? TEXT("  (no numeric deltas)") : (TEXT("  ") + Numerics.TrimStart());
	FString Policy = FString::Printf(TEXT(" Policy=%s"), *UEnum::GetValueAsString(S.RemovalPolicy));
	return Header + Policy + TEXT("\n") + Detail;
}

static FString FormatHitRecord(const FTacLogHitRecord& H)
{
	return FString::Printf(
		TEXT("  Hit %s @ (%d,%d): %s Dmg=%d/%d %s HP_left=%d killed=%s effects=%d"),
		*H.TargetId.ToString(),
		H.TargetCoords.X, H.TargetCoords.Y,
		*UEnum::GetValueAsString(H.Outcome),
		H.DamageResult.Damage, H.DamageResult.DamageBlocked,
		*UEnum::GetValueAsString(H.DamageResult.DamageSource),
		H.RemainingHp,
		H.bKilledTarget ? TEXT("true") : TEXT("false"),
		H.AppliedEffects.Num());
}

static FString FormatStep(const FTacLogStatusChangeStep& S)
{
	const FString ModStr = S.ModifierId.IsValid() ? FString::Printf(TEXT(" Modifier=%s"), *S.ModifierId.ToString()) : TEXT("");
	return FString::Printf(TEXT("[StatusChange] Unit=%s Status=%s %s%s"),
		*S.UnitId.ToString(),
		*UEnum::GetValueAsString(S.Status),
		S.bActivated ? TEXT("SET") : TEXT("CLEARED"),
		*ModStr);
}

static FString FormatStep(const FTacLogCombatStep& S)
{
	FString Result = FString::Printf(TEXT("[Combat] Attacker=%s @ (%d,%d) Target=%s"),
		*S.AttackerId.ToString(),
		S.AttackerCoords.X, S.AttackerCoords.Y,
		*S.PrimaryTargetId.ToString());

	for (const FTacLogHitRecord& H : S.HitRecords)
	{
		Result += TEXT("\n") + FormatHitRecord(H);
	}
	return Result;
}

static FString FormatStep(const FTacLogEffectSpawnStep& S)
{
	return FString::Printf(TEXT("[EffectSpawn] Target=%s Effect=%s Instance=%s"),
		*S.TargetUnitId.ToString(),
		*S.EffectRef.AssetId.ToString(),
		*S.EffectRef.InstanceId.ToString());
}

static FString DispatchFormatStep(const TInstancedStruct<FTacLogStepBase>& Step)
{
	if (const FTacLogWaitStep*         S = Step.GetPtr<FTacLogWaitStep>())         return FormatStep(*S);
	if (const FTacLogFleeStep*         S = Step.GetPtr<FTacLogFleeStep>())         return FormatStep(*S);
	if (const FTacLogMoveStep*         S = Step.GetPtr<FTacLogMoveStep>())         return FormatStep(*S);
	if (const FTacLogStatAltStep*      S = Step.GetPtr<FTacLogStatAltStep>())      return FormatStep(*S);
	if (const FTacLogStatusChangeStep* S = Step.GetPtr<FTacLogStatusChangeStep>()) return FormatStep(*S);
	if (const FTacLogCombatStep*       S = Step.GetPtr<FTacLogCombatStep>())       return FormatStep(*S);
	if (const FTacLogEffectSpawnStep*  S = Step.GetPtr<FTacLogEffectSpawnStep>())  return FormatStep(*S);
	return TEXT("[UnknownStep]");
}

// ── Payload formatters ───────────────────────────────────────────────────────

static FString FormatPayload(const FAbilityUsePayload& P)
{
	FString Result = FString::Printf(TEXT("Ability=%s Instance=%s Cmd=(%d,%d)"),
		*P.AbilityAssetId.ToString(), *P.AbilityInstanceId.ToString(),
		P.Command.X, P.Command.Y);

	for (const TInstancedStruct<FTacLogStepBase>& Step : P.Steps)
	{
		Result += TEXT("\n  ") + DispatchFormatStep(Step);
	}
	return Result;
}

static FString FormatPayload(const FTacEffectPayload& P)
{
	FString Result = FString::Printf(TEXT("Effect=%s Instance=%s Owner=%s Src=%s"),
		*P.EffectAssetId.ToString(), *P.EffectInstanceId.ToString(),
		*P.OwnerUnitId.ToString(), *P.SourceId.ToString());

	for (const TInstancedStruct<FTacLogStepBase>& Step : P.Steps)
	{
		Result += TEXT("\n  ") + DispatchFormatStep(Step);
	}
	return Result;
}

static FString FormatPayload(const FEffectEndPayload& P)
{
	return FString::Printf(TEXT("EffectEnd=%s Instance=%s Owner=%s Reason=%s"),
		*P.EffectAssetId.ToString(), *P.EffectInstanceId.ToString(),
		*P.OwnerUnitId.ToString(),
		*UEnum::GetValueAsString(P.RemovalReason));
}

static FString FormatPayload(const FUnitMoveOffFieldPayload& P)
{
	return FString::Printf(TEXT("UnitExitField: Unit=%s @ (%d,%d) Team=%s Fled=%s"),
		*P.UnitId.ToString(),
		P.LastFieldCoords.X, P.LastFieldCoords.Y,
		*UEnum::GetValueAsString(P.TeamSide),
		P.bFled ? TEXT("true") : TEXT("false"));
}

static FString FormatPayload(const FTurnChangePayload& P)
{
	return FString::Printf(TEXT("TurnChange: %s -> %s Team=%s Kind=%s"),
		*P.OldTurnOwner.ToString(), *P.NewTurnOwner.ToString(),
		*UEnum::GetValueAsString(P.NewTurnTeam),
		*UEnum::GetValueAsString(P.ChangeKind));
}

static FString FormatPayload(const FUnitSpawnPayload& P)
{
	return FString::Printf(TEXT("UnitSpawn: Unit=%s @ (%d,%d) Team=%s Summoner=%s Def=%s"),
		*P.SpawnedUnitId.ToString(),
		P.SpawnCoords.Row, P.SpawnCoords.Col,
		*UEnum::GetValueAsString(P.TeamSide),
		P.SummonerUnitId.IsValid() ? *P.SummonerUnitId.ToString() : TEXT("none"),
		*P.UnitDefinitionId.ToString());
}

static FString DispatchFormatPayload(const TInstancedStruct<FTacLogPayload>& Payload)
{
	if (!Payload.IsValid()) return TEXT("<no payload>");
	if (const FAbilityUsePayload*        P = Payload.GetPtr<FAbilityUsePayload>())        return FormatPayload(*P);
	if (const FTacEffectPayload*         P = Payload.GetPtr<FTacEffectPayload>())         return FormatPayload(*P);
	if (const FEffectEndPayload*         P = Payload.GetPtr<FEffectEndPayload>())         return FormatPayload(*P);
	if (const FUnitMoveOffFieldPayload*  P = Payload.GetPtr<FUnitMoveOffFieldPayload>())  return FormatPayload(*P);
	if (const FTurnChangePayload*        P = Payload.GetPtr<FTurnChangePayload>())        return FormatPayload(*P);
	if (const FUnitSpawnPayload*         P = Payload.GetPtr<FUnitSpawnPayload>())         return FormatPayload(*P);
	return TEXT("<no payload>");
}

// ── Subsystem ────────────────────────────────────────────────────────────────

void UTacLogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FString WorldName = GetWorld()->GetName();
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	OngoingTextLogPath = FPaths::ProjectLogDir() / FString::Printf(TEXT("TacLog_%s_%s.log"), *WorldName, *Timestamp);

	FString Header = FString::Printf(TEXT("=== TacLog opened: %s | World: %s ===\n"),
		*FDateTime::Now().ToString(), *WorldName);
	FFileHelper::SaveStringToFile(Header, *OngoingTextLogPath);
}

void UTacLogSubsystem::Deinitialize()
{
	FString WorldName = GetWorld()->GetName();
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	FString BasePath  = FPaths::ProjectLogDir() / FString::Printf(TEXT("TacLog_%s_%s"), *WorldName, *Timestamp);
	SerializeToFile(BasePath);

	Super::Deinitialize();
}

FGuid UTacLogSubsystem::OpenEvent(ETacLogEventType Type, ETacLogEventOrigin Origin,
	FGuid InstigatorId, FGuid ParentId,
	int32 Round, int32 TurnNumber)
{
	FTacLogEvent Event;
	Event.EventId          = FGuid::NewGuid();
	Event.ParentId         = ParentId;
	Event.Type             = Type;
	Event.Origin           = Origin;
	Event.InstigatorId     = InstigatorId;
	Event.RoundNumber      = (Round == -1)      ? CachedRound      : Round;
	Event.TurnNumber       = (TurnNumber == -1) ? CachedTurnNumber : TurnNumber;
	Event.State            = ETacLogEventState::Open;
	Event.SequencePosition = Spine.Num();

	Spine.Add(Event.EventId);
	EventMap.Add(Event.EventId, Event);
	OpenStack.Add(Event.EventId);

	AppendOpenMarkerToTextLog(Event);

	return Event.EventId;
}

void UTacLogSubsystem::CloseEvent(FGuid EventId, TInstancedStruct<FTacLogPayload> Payload)
{
	FTacLogEvent* Event = EventMap.Find(EventId);
	if (!Event)
	{
		UE_LOG(LogTacLog, Error, TEXT("CloseEvent: EventId %s not found in EventMap"), *EventId.ToString());
		return;
	}

	int32 StackIndex = INDEX_NONE;
	for (int32 i = OpenStack.Num() - 1; i >= 0; --i)
	{
		if (OpenStack[i] == EventId)
		{
			StackIndex = i;
			break;
		}
	}

	if (StackIndex == INDEX_NONE)
	{
		UE_LOG(LogTacLog, Error, TEXT("CloseEvent: EventId %s not in OpenStack (already closed or invalid)"), *EventId.ToString());
		return;
	}

	for (int32 i = OpenStack.Num() - 1; i > StackIndex; --i)
	{
		const FTacLogEvent* NestedEvent = EventMap.Find(OpenStack[i]);
		if (NestedEvent && NestedEvent->State == ETacLogEventState::Open)
		{
			UE_LOG(LogTacLog, Warning,
				TEXT("CloseEvent: closing %s while nested event %s (Type=%s) is still open"),
				*EventId.ToString(),
				*NestedEvent->EventId.ToString(),
				*UEnum::GetValueAsString(NestedEvent->Type));
		}
	}

	Event->State   = ETacLogEventState::Closed;
	Event->Payload = MoveTemp(Payload);

	if (const FTurnChangePayload* TurnPayload = Event->Payload.GetPtr<FTurnChangePayload>())
	{
		if (TurnPayload->ChangeKind == ETurnChangeKind::Round)
		{
			CachedRound++;
			CachedTurnNumber = 0;
		}
		else
		{
			CachedTurnNumber++;
		}
	}

	OpenStack.RemoveAt(StackIndex);

	AppendCloseEntryToTextLog(*Event);
}

const FTacLogEvent* UTacLogSubsystem::GetEvent(FGuid EventId) const
{
	return EventMap.Find(EventId);
}

FGuid UTacLogSubsystem::FindClosestEvent(ETacLogEventType Type) const
{
	for (int32 i = OpenStack.Num() - 1; i >= 0; --i)
	{
		const FTacLogEvent* Event = EventMap.Find(OpenStack[i]);
		if (Event && Event->Type == Type)
		{
			return OpenStack[i];
		}
	}
	return FGuid();
}

FGuid UTacLogSubsystem::FindLatestEvent() const
{
	if (OpenStack.Num() > 0)
	{
		return OpenStack.Last();
	}
	return FGuid();
}

void UTacLogSubsystem::SerializeToFile(const FString& BasePath) const
{
	// Binary archive
	FString BinaryPath = BasePath + TEXT(".bin");
	{
		TArray<uint8> Bytes;
		FMemoryWriter Ar(Bytes, /*bIsPersistent=*/true);
		int32 Count = Spine.Num();
		Ar << Count;
		for (const FGuid& Id : Spine)
		{
			checkf(EventMap.Contains(Id), TEXT("SerializeToFile: Spine guid %s missing from EventMap"), *Id.ToString());
			FTacLogEvent EventCopy = *EventMap.Find(Id);
			FTacLogEvent::StaticStruct()->SerializeBin(Ar, &EventCopy);
		}
		FFileHelper::SaveArrayToFile(Bytes, *BinaryPath);
	}

	// Full text snapshot
	FString LogPath = BasePath + TEXT(".log");
	FString TextOutput;
	for (const FGuid& Id : Spine)
	{
		checkf(EventMap.Contains(Id), TEXT("SerializeToFile: Spine guid %s missing from EventMap"), *Id.ToString());
		const FTacLogEvent& Event = *EventMap.Find(Id);

		FString StateStr  = (Event.State == ETacLogEventState::Open) ? TEXT("OPEN") : TEXT("CLOSED");
		FString ParentStr = Event.ParentId.IsValid() ? Event.ParentId.ToString() : TEXT("none");
		FString PayloadText = DispatchFormatPayload(Event.Payload);

		// Indent payload lines by 4 spaces
		TArray<FString> PayloadLines;
		PayloadText.ParseIntoArrayLines(PayloadLines, /*bCullEmpty=*/false);
		FString IndentedPayload;
		for (const FString& Line : PayloadLines)
		{
			IndentedPayload += TEXT("    ") + Line + TEXT("\n");
		}

		TextOutput += FString::Printf(
			TEXT("── [%s] R%d:T%d | %s | %s | %s\n")
			TEXT("   Parent:     %s\n")
			TEXT("   Instigator: %s\n")
			TEXT("   State:      %s\n")
			TEXT("   Payload:\n")
			TEXT("%s\n"),
			*StateStr,
			Event.RoundNumber, Event.TurnNumber,
			*UEnum::GetValueAsString(Event.Type),
			*UEnum::GetValueAsString(Event.Origin),
			*Event.EventId.ToString(),
			*ParentStr,
			*Event.InstigatorId.ToString(),
			*StateStr,
			*IndentedPayload);
	}
	FFileHelper::SaveStringToFile(TextOutput, *LogPath);
}

bool UTacLogSubsystem::LoadFromFile(const FString& BinaryPath)
{
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *BinaryPath)) return false;

	FMemoryReader Ar(Bytes, /*bIsPersistent=*/true);
	Spine.Reset();
	EventMap.Reset();

	int32 Count;
	Ar << Count;
	for (int32 i = 0; i < Count; ++i)
	{
		FTacLogEvent Event;
		FTacLogEvent::StaticStruct()->SerializeBin(Ar, &Event);
		Spine.Add(Event.EventId);
		EventMap.Add(Event.EventId, MoveTemp(Event));
	}
	return !Ar.IsError();
}

FString UTacLogSubsystem::FormatEventHeader(const FTacLogEvent& Event) const
{
	return FString::Printf(TEXT("R%d:T%d | %s | %s"),
		Event.RoundNumber,
		Event.TurnNumber,
		*UEnum::GetValueAsString(Event.Type),
		*Event.EventId.ToString());
}

void UTacLogSubsystem::AppendOpenMarkerToTextLog(const FTacLogEvent& Event)
{
	FString Line = FString::Printf(TEXT("[OPEN]  %s | Instigator=%s\n"),
		*FormatEventHeader(Event),
		*Event.InstigatorId.ToString());

	FFileHelper::SaveStringToFile(Line, *OngoingTextLogPath, FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(), FILEWRITE_Append);
}

void UTacLogSubsystem::AppendCloseEntryToTextLog(const FTacLogEvent& Event)
{
	FString PayloadText = DispatchFormatPayload(Event.Payload);

	TArray<FString> PayloadLines;
	PayloadText.ParseIntoArrayLines(PayloadLines, /*bCullEmpty=*/false);
	FString IndentedPayload;
	for (const FString& Line : PayloadLines)
	{
		IndentedPayload += TEXT("  ") + Line + TEXT("\n");
	}

	FString Block = FString::Printf(TEXT("[CLOSE] %s | State=Closed\n%s"),
		*FormatEventHeader(Event),
		*IndentedPayload);

	FFileHelper::SaveStringToFile(Block, *OngoingTextLogPath, FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(), FILEWRITE_Append);
}

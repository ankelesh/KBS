// File: Source/KBS/Private/GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.cpp
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacSubsystemControl.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPrettyPrint.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Serialization/MemoryReader.h"

DEFINE_LOG_CATEGORY_STATIC(LogTacLog, Log, All);

// ── Subsystem ─────────────────────────────────────────────────────────────────

void UTacLogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FString WorldName = GetWorld()->GetName();
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	LogBasePath = FPaths::ProjectLogDir() / FString::Printf(TEXT("TacLog_%s_%s"), *WorldName, *Timestamp);

	UTacSubsystemControl* Control = Collection.InitializeDependency<UTacSubsystemControl>();
	checkf(Control, TEXT("UTacLogSubsystem: TacSubsystemControl unavailable"));
	Control->ReadyForStart.AddDynamic(this, &UTacLogSubsystem::OnSubsystemsReady);
}

void UTacLogSubsystem::Deinitialize()
{
	FinalizeLog();
	Super::Deinitialize();
}

void UTacLogSubsystem::FinalizeLog()
{
	if (bFinalized) return;
	bFinalized = true;

	SerializeToFile(LogBasePath);
}

void UTacLogSubsystem::OnSubsystemsReady()
{
	UTacSubsystemControl* Control = GetWorld()->GetSubsystem<UTacSubsystemControl>();
	if (Control)
	{
		Control->ReadyForStart.RemoveDynamic(this, &UTacLogSubsystem::OnSubsystemsReady);
	}

	UTacTurnSubsystem* TurnSubsystem = GetWorld()->GetSubsystem<UTacTurnSubsystem>();
	checkf(TurnSubsystem, TEXT("UTacLogSubsystem: TacTurnSubsystem unavailable"));
	TurnSubsystem->OnBattleEnd.AddDynamic(this, &UTacLogSubsystem::HandleBattleEnd);
}

void UTacLogSubsystem::HandleBattleEnd(UBattleTeam* Winner)
{
	FinalizeLog();
}

FString UTacLogSubsystem::RegisterUnit(FGuid UnitId, const FString& UnitTypeName)
{
	if (UnitNameRegistry.Contains(UnitId))
		return UnitNameRegistry[UnitId];

	int32 Index = UnitTypeCounters.FindOrAdd(UnitTypeName, 0);
	UnitTypeCounters[UnitTypeName] = Index + 1;

	FString Label = MakeUnitLabel(UnitTypeName, Index);
	UnitNameRegistry.Add(UnitId, Label);
	return Label;
}

FString UTacLogSubsystem::ShortUnit(FGuid Id) const
{
	const FString* Name = UnitNameRegistry.Find(Id);
	if (Name) return *Name;
	return Id.IsValid() ? TEXT("?unit") : TEXT("-");
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
	Event.Depth            = OpenStack.Num();

	Spine.Add(Event.EventId);
	EventMap.Add(Event.EventId, Event);
	OpenStack.Add(Event.EventId);

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

	if (Event->Payload.GetPtr<FTurnChangePayload>())
	{
		CachedRound      = Event->RoundNumber;
		CachedTurnNumber = Event->TurnNumber;
	}

	OpenStack.RemoveAt(StackIndex);
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
			return OpenStack[i];
	}
	return FGuid();
}

FGuid UTacLogSubsystem::FindLatestEvent() const
{
	if (OpenStack.Num() > 0)
		return OpenStack.Last();
	return FGuid();
}

FString UTacLogSubsystem::FormatEventHeader(const FTacLogEvent& Event) const
{
	return FString::Printf(TEXT("R%d:T%d  %s:%s  %s"),
		Event.RoundNumber, Event.TurnNumber,
		*ShortType(Event.Type), *ShortOrigin(Event.Origin),
		*ShortUnit(Event.InstigatorId));
}


void UTacLogSubsystem::SerializeToFile(const FString& BasePath) const
{
	FTacLogFormatCtx Ctx{UnitNameRegistry};

	FString TextOutput = BuildTacLogLegend();

	for (const FGuid& Id : Spine)
	{
		checkf(EventMap.Contains(Id), TEXT("SerializeToFile: Spine guid %s missing from EventMap"), *Id.ToString());
		const FTacLogEvent& Event = *EventMap.Find(Id);

		FString Indent    = FString::ChrN(Event.Depth * 2, TEXT(' '));
		bool    bIsClosed = (Event.State == ETacLogEventState::Closed);
		FString StateTag  = bIsClosed ? TEXT("C") : TEXT("!");

		TextOutput += FString::Printf(TEXT("%s%s  %s  |#%d\n"),
			*Indent, *StateTag, *FormatEventHeader(Event), Event.SequencePosition);

		if (bIsClosed)
		{
			FString PayloadText   = DispatchFormatPayload(Event.Payload, Ctx);
			FString PayloadIndent = Indent + TEXT("   ");

			TArray<FString> Lines;
			PayloadText.ParseIntoArrayLines(Lines, /*bCullEmpty=*/false);
			for (const FString& PL : Lines)
				TextOutput += PayloadIndent + PL + TEXT("\n");
		}
		TextOutput += TEXT("\n");
	}

	FFileHelper::SaveStringToFile(TextOutput, *(BasePath + TEXT(".log")));
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

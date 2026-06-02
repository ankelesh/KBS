#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogEvent.h"
#include "TacLogSubsystem.generated.h"

UCLASS()
class KBS_API UTacLogSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Allocates event in spine+map, pushes to open stack, appends open-marker to text log.
	// Caller owns ParentId assignment (pass FGuid() for no parent).
	FGuid OpenEvent(ETacLogEventType Type, ETacLogEventOrigin Origin,
	                FGuid InstigatorId, FGuid ParentId,
	                int32 RoundNumber, int32 TurnNumber);

	// Seals event: inserts payload, marks Closed, pops from open stack.
	// UE_LOG error if events opened after this one are still open (nested not closed).
	void CloseEvent(FGuid EventId, TInstancedStruct<FTacLogPayload> Payload);

	// O(1) map lookup. Returns nullptr if not found.
	const FTacLogEvent* GetEvent(FGuid EventId) const;

	// Walk open stack top-down; return first EventId whose Type matches. Invalid guid if none.
	FGuid FindClosestEvent(ETacLogEventType Type) const;

	// Return top of open stack (most recently opened). Invalid guid if stack is empty.
	FGuid FindLatestEvent() const;

	// Full spine-ordered dump of all events (including open) to:
	//   <BasePath>.bin  — binary FArchive dump, fully deserializable
	//   <BasePath>.log  — complete pretty-printed text snapshot
	void SerializeToFile(const FString& BasePath) const;

	// Rebuilds Spine + EventMap from a .bin file produced by SerializeToFile.
	bool LoadFromFile(const FString& BinaryPath);

private:
	void AppendOpenMarkerToTextLog(const FTacLogEvent& Event);
	void AppendCloseEntryToTextLog(const FTacLogEvent& Event);
	FString FormatEventHeader(const FTacLogEvent& Event) const;

	UPROPERTY()
	TArray<FGuid> Spine; // insertion-ordered guid sequence

	UPROPERTY()
	TMap<FGuid, FTacLogEvent> EventMap;

	TArray<FGuid> OpenStack; // back = top (most recently opened)

	FString OngoingTextLogPath; // set in Initialize, appended per event
};

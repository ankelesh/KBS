#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogEvent.h"
#include "TacLogSubsystem.generated.h"

class UBattleTeam;

UCLASS()
class KBS_API UTacLogSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Allocates event in spine+map, pushes to open stack, appends open-marker to text log.
	// Caller owns ParentId assignment (pass FGuid() for no parent).
	// Round/TurnNumber default to -1: subsystem fills from cache. Pass explicit values only
	// when the caller controls the transition (e.g. TurnChange events).
	FGuid OpenEvent(ETacLogEventType Type, ETacLogEventOrigin Origin,
	                FGuid InstigatorId, FGuid ParentId,
	                int32 Round = -1, int32 TurnNumber = -1);

	// Seals event: inserts payload, marks Closed, pops from open stack.
	// UE_LOG error if events opened after this one are still open (nested not closed).
	void CloseEvent(FGuid EventId, TInstancedStruct<FTacLogPayload> Payload);

	// O(1) map lookup. Returns nullptr if not found.
	const FTacLogEvent* GetEvent(FGuid EventId) const;

	// Insertion-ordered guid sequence. Callers slice ranges with IndexOfByKey.
	const TArray<FGuid>& GetSpine() const { return Spine; }

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

	// Registers a unit for log display. Call at spawn time with UnitDef->UnitName.
	// Returns the assigned label, e.g. "Warrior-alpha". Safe to call multiple times for same guid.
	FString RegisterUnit(FGuid UnitId, const FString& UnitTypeName);

	// Writes footer to text log and serializes binary+text snapshots. Idempotent.
	void FinalizeLog();

private:
	UFUNCTION()
	void HandleBattleEnd(UBattleTeam* Winner);

	UFUNCTION()
	void OnSubsystemsReady();

	FString FormatEventHeader(const FTacLogEvent& Event) const;
	FString ShortUnit(FGuid Id) const;

	UPROPERTY()
	TArray<FGuid> Spine; // insertion-ordered guid sequence

	UPROPERTY()
	TMap<FGuid, FTacLogEvent> EventMap;

	TArray<FGuid> OpenStack; // back = top (most recently opened)

	FString LogBasePath; // ProjectLogDir/TacLog_<World>_<Timestamp>, no extension
	bool bFinalized = false;

	int32 CachedRound = 0;
	int32 CachedTurnNumber = 0;

	TMap<FGuid, FString> UnitNameRegistry;   // UnitId → "Warrior-alpha"
	TMap<FString, int32> UnitTypeCounters;   // TypeName → next name index
};

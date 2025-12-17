#include "GameMechanics/Tactical/Grid/Components/PresentationTrackerComponent.h"

UPresentationTrackerComponent::UPresentationTrackerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	ActiveBatches = 0;
	bCheckIdleNextFrame = false;
}

void UPresentationTrackerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPresentationTrackerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bCheckIdleNextFrame)
	{
		bCheckIdleNextFrame = false;

		// Re-check IsIdle() to allow 1 frame for new operations to register
		if (IsIdle())
		{
			UE_LOG(LogTemp, Log, TEXT("PresentationTracker: All operations complete, broadcasting event"));
			OnAllOperationsComplete.Broadcast();
		}
	}
}

FOperationHandle UPresentationTrackerComponent::RegisterOperation(const FString& DebugLabel)
{
	FGuid NewGuid = FGuid::NewGuid();
	PendingOperations.Add(NewGuid, DebugLabel);

	UE_LOG(LogTemp, Log, TEXT("PresentationTracker: Registered operation '%s' [%s] (Total: %d)"),
		*DebugLabel, *NewGuid.ToString(), PendingOperations.Num());

	return FOperationHandle(NewGuid);
}

void UPresentationTrackerComponent::UnregisterOperation(FOperationHandle Handle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("PresentationTracker: Attempted to unregister invalid handle"));
		return;
	}

	FString* DebugLabel = PendingOperations.Find(Handle.ID);
	if (DebugLabel)
	{
		UE_LOG(LogTemp, Log, TEXT("PresentationTracker: Unregistered operation '%s' [%s] (Remaining: %d)"),
			**DebugLabel, *Handle.ID.ToString(), PendingOperations.Num() - 1);

		PendingOperations.Remove(Handle.ID);

		// If map is now empty AND no active batches, defer idle check to next frame
		if (PendingOperations.Num() == 0 && ActiveBatches == 0)
		{
			bCheckIdleNextFrame = true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PresentationTracker: Handle not found in pending operations [%s]"), *Handle.ID.ToString());
	}
}

bool UPresentationTrackerComponent::IsIdle() const
{
	return PendingOperations.Num() == 0 && ActiveBatches == 0;
}

void UPresentationTrackerComponent::BeginBatch(const FString& BatchName)
{
	ActiveBatches++;
	UE_LOG(LogTemp, Log, TEXT("PresentationTracker: Begin batch '%s' (Active batches: %d)"), *BatchName, ActiveBatches);
}

void UPresentationTrackerComponent::EndBatch()
{
	ActiveBatches--;
	check(ActiveBatches >= 0);

	UE_LOG(LogTemp, Log, TEXT("PresentationTracker: End batch (Active batches: %d, Pending ops: %d)"),
		ActiveBatches, PendingOperations.Num());

	// If now idle, defer check to next frame
	if (IsIdle())
	{
		bCheckIdleNextFrame = true;
	}
}

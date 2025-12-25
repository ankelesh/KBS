#include "GameMechanics/Tactical/PresentationSubsystem.h"
#include "Engine/World.h"

void UPresentationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("PresentationSubsystem: Initialized"));

	// Create a default batch for operations without explicit batch
	DefaultBatch = BeginBatch("DefaultBatch");
}

void UPresentationSubsystem::Deinitialize()
{
	ActiveBatches.Empty();
	PendingOperations.Empty();
	UE_LOG(LogTemp, Log, TEXT("PresentationSubsystem: Deinitialized"));
	Super::Deinitialize();
}

UPresentationSubsystem* UPresentationSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("PresentationSubsystem::Get - WorldContextObject is null"));
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("PresentationSubsystem::Get - World is null"));
		return nullptr;
	}

	return World->GetSubsystem<UPresentationSubsystem>();
}

FBatchHandle UPresentationSubsystem::BeginBatch(const FString& BatchName)
{
	FGuid NewGuid = FGuid::NewGuid();
	FBatchHandle Handle(NewGuid);

	FPresentationBatch& Batch = ActiveBatches.Add(NewGuid);
	Batch.Name = BatchName;
	Batch.bEnded = false;

	UE_LOG(LogTemp, Log, TEXT("PresentationSubsystem: Begin batch '%s' [%s] (Active batches: %d)"),
		*BatchName, *NewGuid.ToString(), ActiveBatches.Num());

	return Handle;
}

void UPresentationSubsystem::EndBatch(FBatchHandle BatchHandle)
{
	if (!BatchHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("PresentationSubsystem: Attempted to end invalid batch"));
		return;
	}

	FPresentationBatch* Batch = ActiveBatches.Find(BatchHandle.ID);
	if (!Batch)
	{
		UE_LOG(LogTemp, Warning, TEXT("PresentationSubsystem: Batch not found [%s]"), *BatchHandle.ID.ToString());
		return;
	}

	Batch->bEnded = true;
	UE_LOG(LogTemp, Log, TEXT("PresentationSubsystem: End batch '%s' (Pending ops: %d)"),
		*Batch->Name, Batch->PendingOperations.Num());

	// Check if batch is immediately complete
	if (Batch->IsComplete())
	{
		OnBatchCompleted(BatchHandle);
	}
}

FOperationHandle UPresentationSubsystem::RegisterOperation(const FString& DebugLabel, FBatchHandle BatchHandle)
{
	// If no batch specified, use current active batch or default batch
	if (!BatchHandle.IsValid())
	{
		BatchHandle = GetCurrentBatch();
		if (!BatchHandle.IsValid())
		{
			BatchHandle = DefaultBatch;
		}
	}

	FPresentationBatch* Batch = ActiveBatches.Find(BatchHandle.ID);
	if (!Batch)
	{
		UE_LOG(LogTemp, Warning, TEXT("PresentationSubsystem: Cannot register operation - batch not found [%s]"),
			*BatchHandle.ID.ToString());
		return FOperationHandle();
	}

	FGuid NewGuid = FGuid::NewGuid();
	FOperationHandle Handle(NewGuid);

	FPresentationOperation& Operation = PendingOperations.Add(NewGuid);
	Operation.DebugLabel = DebugLabel;
	Operation.OwningBatch = BatchHandle;

	Batch->PendingOperations.Add(Handle);

	UE_LOG(LogTemp, Log, TEXT("PresentationSubsystem: Registered operation '%s' [%s] in batch '%s' (Total ops: %d)"),
		*DebugLabel, *NewGuid.ToString(), *Batch->Name, PendingOperations.Num());

	return Handle;
}

void UPresentationSubsystem::UnregisterOperation(FOperationHandle Handle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("PresentationSubsystem: Attempted to unregister invalid handle"));
		return;
	}

	FPresentationOperation* Operation = PendingOperations.Find(Handle.ID);
	if (!Operation)
	{
		UE_LOG(LogTemp, Warning, TEXT("PresentationSubsystem: Handle not found [%s]"), *Handle.ID.ToString());
		return;
	}

	FBatchHandle BatchHandle = Operation->OwningBatch;
	FString DebugLabel = Operation->DebugLabel;

	// Remove from batch's pending operations
	FPresentationBatch* Batch = ActiveBatches.Find(BatchHandle.ID);
	if (Batch)
	{
		Batch->PendingOperations.Remove(Handle);
	}

	// Remove operation
	PendingOperations.Remove(Handle.ID);

	UE_LOG(LogTemp, Log, TEXT("PresentationSubsystem: Unregistered operation '%s' [%s] (Remaining ops: %d)"),
		*DebugLabel, *Handle.ID.ToString(), PendingOperations.Num());

	// Check if batch is now complete
	if (Batch && Batch->IsComplete())
	{
		OnBatchCompleted(BatchHandle);
	}
}

bool UPresentationSubsystem::IsIdle() const
{
	// Idle means no active batches with pending operations
	// DefaultBatch is persistent and never ends, so we ignore it
	for (const auto& Pair : ActiveBatches)
	{
		const FPresentationBatch& Batch = Pair.Value;

		// Skip DefaultBatch - it's persistent and never ends
		if (Batch.Name == TEXT("DefaultBatch"))
		{
			continue;
		}

		if (!Batch.IsComplete())
		{
			return false;
		}
	}
	return true;
}

bool UPresentationSubsystem::IsBatchComplete(FBatchHandle BatchHandle) const
{
	if (!BatchHandle.IsValid())
	{
		return true;
	}

	const FPresentationBatch* Batch = ActiveBatches.Find(BatchHandle.ID);
	if (!Batch)
	{
		return true; // Batch doesn't exist = complete
	}

	return Batch->IsComplete();
}

void UPresentationSubsystem::CheckAndBroadcastIdle()
{
	if (IsIdle())
	{
		UE_LOG(LogTemp, Log, TEXT("PresentationSubsystem: All presentations complete, broadcasting event"));
		OnAllPresentationsComplete.Broadcast();
	}
}

void UPresentationSubsystem::OnBatchCompleted(FBatchHandle BatchHandle)
{
	FPresentationBatch* Batch = ActiveBatches.Find(BatchHandle.ID);
	if (!Batch)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("PresentationSubsystem: Batch '%s' completed"), *Batch->Name);

	// Remove completed batch
	ActiveBatches.Remove(BatchHandle.ID);

	// Broadcast batch complete
	OnBatchCompleteNative.Broadcast(BatchHandle);

	// Check if all presentations are now idle
	CheckAndBroadcastIdle();
}

// RAII Helper Implementation - FScopedOperation
UPresentationSubsystem::FScopedOperation::FScopedOperation(UPresentationSubsystem* InSubsystem, const FString& DebugLabel, FBatchHandle BatchHandle)
	: Subsystem(InSubsystem)
	, OperationHandle()
	, bCompleted(false)
{
	if (Subsystem)
	{
		OperationHandle = Subsystem->RegisterOperation(DebugLabel, BatchHandle);
	}
}

UPresentationSubsystem::FScopedOperation::~FScopedOperation()
{
	Complete();
}

void UPresentationSubsystem::FScopedOperation::Complete()
{
	if (!bCompleted && Subsystem && OperationHandle.IsValid())
	{
		Subsystem->UnregisterOperation(OperationHandle);
		bCompleted = true;
	}
}

// RAII Helper Implementation - FScopedBatch
UPresentationSubsystem::FScopedBatch::FScopedBatch(UPresentationSubsystem* InSubsystem, const FString& BatchName)
	: Subsystem(InSubsystem)
	, BatchHandle()
{
	if (Subsystem)
	{
		BatchHandle = Subsystem->BeginBatch(BatchName);
		Subsystem->PushActiveBatch(BatchHandle);
	}
}

UPresentationSubsystem::FScopedBatch::~FScopedBatch()
{
	if (Subsystem && BatchHandle.IsValid())
	{
		Subsystem->PopActiveBatch();
		Subsystem->EndBatch(BatchHandle);
	}
}

FOperationHandle UPresentationSubsystem::FScopedBatch::RegisterOperation(const FString& DebugLabel)
{
	if (!Subsystem || !BatchHandle.IsValid())
	{
		return FOperationHandle();
	}

	return Subsystem->RegisterOperation(DebugLabel, BatchHandle);
}

TSharedPtr<UPresentationSubsystem::FScopedOperation> UPresentationSubsystem::FScopedBatch::CreateScopedOperation(const FString& DebugLabel)
{
	if (!Subsystem || !BatchHandle.IsValid())
	{
		return nullptr;
	}

	return MakeShared<FScopedOperation>(Subsystem, DebugLabel, BatchHandle);
}

// Context management implementation
void UPresentationSubsystem::PushActiveBatch(FBatchHandle BatchHandle)
{
	ActiveBatchStack.Push(BatchHandle);
}

void UPresentationSubsystem::PopActiveBatch()
{
	if (ActiveBatchStack.Num() > 0)
	{
		ActiveBatchStack.Pop();
	}
}

FBatchHandle UPresentationSubsystem::GetCurrentBatch() const
{
	return ActiveBatchStack.Num() > 0 ? ActiveBatchStack.Last() : FBatchHandle();
}

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PresentationTrackerComponent.generated.h"

/**
 * Lightweight GUID wrapper representing a pending presentation operation
 */
USTRUCT(BlueprintType)
struct FOperationHandle
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid ID;

	bool IsValid() const { return ID.IsValid(); }

	FOperationHandle() : ID() {}
	explicit FOperationHandle(const FGuid& InID) : ID(InID) {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllOperationsComplete);

/**
 * Tracks async presentation operations (animations, VFX, SFX) to ensure turn advancement
 * only occurs after all visual/audio feedback completes.
 *
 * Uses handle-based operation tracking with lambda callbacks for clean async management.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KBS_API UPresentationTrackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPresentationTrackerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Registers a new pending operation and returns a handle for later cleanup
	 * @param DebugLabel - Human-readable label for tracking what's pending
	 * @return Handle to pass to UnregisterOperation when operation completes
	 */
	UFUNCTION(BlueprintCallable, Category = "Presentation Tracker")
	FOperationHandle RegisterOperation(const FString& DebugLabel);

	/**
	 * Unregisters a completed operation
	 * @param Handle - The handle returned from RegisterOperation
	 */
	UFUNCTION(BlueprintCallable, Category = "Presentation Tracker")
	void UnregisterOperation(FOperationHandle Handle);

	/**
	 * Checks if all operations are complete and no batches are open
	 * @return True when no pending operations and no active batches
	 */
	UFUNCTION(BlueprintCallable, Category = "Presentation Tracker")
	bool IsIdle() const;

	/**
	 * Begins a batch transaction to prevent premature completion signals
	 * @param BatchName - Debug label for this batch
	 */
	UFUNCTION(BlueprintCallable, Category = "Presentation Tracker")
	void BeginBatch(const FString& BatchName);

	/**
	 * Ends a batch transaction, may trigger completion event if now idle
	 */
	UFUNCTION(BlueprintCallable, Category = "Presentation Tracker")
	void EndBatch();

	/** Fires when all operations complete AND no batches are open */
	UPROPERTY(BlueprintAssignable, Category = "Presentation Tracker")
	FOnAllOperationsComplete OnAllOperationsComplete;

protected:
	virtual void BeginPlay() override;

private:
	/** Maps operation GUIDs to debug labels for tracking what's pending */
	UPROPERTY()
	TMap<FGuid, FString> PendingOperations;

	/** Counter for open batch transactions - prevents premature "all complete" signals */
	UPROPERTY()
	int32 ActiveBatches;

	/** Deferred idle check flag for next tick */
	UPROPERTY()
	bool bCheckIdleNextFrame;
};

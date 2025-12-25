#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PresentationSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FOperationHandle
{
	GENERATED_BODY()
	UPROPERTY()
	FGuid ID;
	bool IsValid() const { return ID.IsValid(); }
	FOperationHandle() : ID() {}
	explicit FOperationHandle(const FGuid& InID) : ID(InID) {}
	bool operator==(const FOperationHandle& Other) const { return ID == Other.ID; }
	friend uint32 GetTypeHash(const FOperationHandle& Handle) { return GetTypeHash(Handle.ID); }
};

USTRUCT(BlueprintType)
struct FBatchHandle
{
	GENERATED_BODY()
	UPROPERTY()
	FGuid ID;
	bool IsValid() const { return ID.IsValid(); }
	FBatchHandle() : ID() {}
	explicit FBatchHandle(const FGuid& InID) : ID(InID) {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllPresentationsComplete);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBatchCompleteNative, FBatchHandle);

// Forward declarations for internal structs
USTRUCT()
struct FPresentationOperation
{
	GENERATED_BODY()

	UPROPERTY()
	FString DebugLabel;

	FBatchHandle OwningBatch;
};

USTRUCT()
struct FPresentationBatch
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	TSet<FOperationHandle> PendingOperations;

	UPROPERTY()
	bool bEnded = false;

	bool IsComplete() const { return bEnded && PendingOperations.IsEmpty(); }
};

UCLASS()
class KBS_API UPresentationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Easy access from anywhere in the world
	static UPresentationSubsystem* Get(const UObject* WorldContextObject);

	// Batch API - for grouping simultaneous operations (e.g., AoE attacks)
	UFUNCTION(BlueprintCallable, Category = "Presentation")
	FBatchHandle BeginBatch(const FString& BatchName);

	UFUNCTION(BlueprintCallable, Category = "Presentation")
	void EndBatch(FBatchHandle BatchHandle);

	// Operation API - for individual async operations
	UFUNCTION(BlueprintCallable, Category = "Presentation")
	FOperationHandle RegisterOperation(const FString& DebugLabel, FBatchHandle BatchHandle = FBatchHandle());

	UFUNCTION(BlueprintCallable, Category = "Presentation")
	void UnregisterOperation(FOperationHandle Handle);

	// Query API
	UFUNCTION(BlueprintCallable, Category = "Presentation")
	bool IsIdle() const;

	UFUNCTION(BlueprintCallable, Category = "Presentation")
	bool IsBatchComplete(FBatchHandle BatchHandle) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Presentation")
	FOnAllPresentationsComplete OnAllPresentationsComplete;

	FOnBatchCompleteNative OnBatchCompleteNative;

	// RAII Helper for automatic single operation management
	class FScopedOperation
	{
	public:
		FScopedOperation(UPresentationSubsystem* InSubsystem, const FString& DebugLabel, FBatchHandle BatchHandle = FBatchHandle());
		~FScopedOperation();

		// Manually complete the operation early (optional)
		void Complete();

		FOperationHandle GetHandle() const { return OperationHandle; }
		bool IsValid() const { return OperationHandle.IsValid() && !bCompleted; }

	private:
		UPresentationSubsystem* Subsystem;
		FOperationHandle OperationHandle;
		bool bCompleted = false;
	};

	// RAII Helper for automatic batch management
	class FScopedBatch
	{
	public:
		FScopedBatch(UPresentationSubsystem* InSubsystem, const FString& BatchName);
		~FScopedBatch();

		// Register an operation within this batch
		FOperationHandle RegisterOperation(const FString& DebugLabel);

		// Create a scoped operation within this batch (RAII for operations too)
		TSharedPtr<FScopedOperation> CreateScopedOperation(const FString& DebugLabel);

		FBatchHandle GetHandle() const { return BatchHandle; }

	private:
		UPresentationSubsystem* Subsystem;
		FBatchHandle BatchHandle;
	};

private:
	UPROPERTY()
	TMap<FGuid, FPresentationBatch> ActiveBatches;

	UPROPERTY()
	TMap<FGuid, FPresentationOperation> PendingOperations;

	// Default batch for operations registered without explicit batch
	FBatchHandle DefaultBatch;

	// Stack of active batch contexts (for nested scopes)
	TArray<FBatchHandle> ActiveBatchStack;

	void CheckAndBroadcastIdle();
	void OnBatchCompleted(FBatchHandle BatchHandle);

	// Internal: Push/pop active batch context (used by FScopedBatch)
	void PushActiveBatch(FBatchHandle BatchHandle);
	void PopActiveBatch();
	FBatchHandle GetCurrentBatch() const;

	friend class FScopedBatch;
};

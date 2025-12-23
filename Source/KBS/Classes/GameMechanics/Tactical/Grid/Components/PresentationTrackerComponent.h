#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PresentationTrackerComponent.generated.h"
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
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KBS_API UPresentationTrackerComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UPresentationTrackerComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(BlueprintCallable, Category = "Presentation Tracker")
	FOperationHandle RegisterOperation(const FString& DebugLabel);
	UFUNCTION(BlueprintCallable, Category = "Presentation Tracker")
	void UnregisterOperation(FOperationHandle Handle);
	UFUNCTION(BlueprintCallable, Category = "Presentation Tracker")
	bool IsIdle() const;
	UFUNCTION(BlueprintCallable, Category = "Presentation Tracker")
	void BeginBatch(const FString& BatchName);
	UFUNCTION(BlueprintCallable, Category = "Presentation Tracker")
	void EndBatch();
	void SetInputLockComponent(class UGridInputLockComponent* InInputLockComponent);
	UPROPERTY(BlueprintAssignable, Category = "Presentation Tracker")
	FOnAllOperationsComplete OnAllOperationsComplete;
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	TMap<FGuid, FString> PendingOperations;
	UPROPERTY()
	int32 ActiveBatches;
	UPROPERTY()
	bool bCheckIdleNextFrame;
	UPROPERTY()
	TObjectPtr<class UGridInputLockComponent> InputLockComponent;
};

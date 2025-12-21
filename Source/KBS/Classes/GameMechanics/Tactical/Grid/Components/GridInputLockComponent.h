#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridInputLockComponent.generated.h"

UENUM(BlueprintType)
enum class EInputLockSource : uint8
{
	TurnTransition,
	AbilityExecution,
	PresentationPlaying,
	AIThinking,
	MovementProcessing
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputLockChanged, bool, bIsLocked);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KBS_API UGridInputLockComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGridInputLockComponent();

	// Lock management
	void RequestLock(EInputLockSource Source);
	void ReleaseLock(EInputLockSource Source);
	void ForceUnlockAll();

	// Query
	UFUNCTION(BlueprintCallable, Category = "Input Lock")
	bool IsLocked() const;

	UFUNCTION(BlueprintCallable, Category = "Input Lock")
	bool IsLockedBy(EInputLockSource Source) const;

	UFUNCTION(BlueprintCallable, Category = "Input Lock")
	FString GetLockDebugInfo() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Input Lock")
	FOnInputLockChanged OnInputLockChanged;

private:
	UPROPERTY()
	TMap<EInputLockSource, int32> LockCounts;

	void BroadcastLockStateChange();
	FString LockSourceToString(EInputLockSource Source) const;
};

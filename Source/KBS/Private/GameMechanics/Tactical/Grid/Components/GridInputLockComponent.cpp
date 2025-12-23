#include "GameMechanics/Tactical/Grid/Components/GridInputLockComponent.h"
UGridInputLockComponent::UGridInputLockComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UGridInputLockComponent::RequestLock(EInputLockSource Source)
{
	bool bWasLocked = IsLocked();
	int32& Count = LockCounts.FindOrAdd(Source, 0);
	++Count;
	UE_LOG(LogTemp, Log, TEXT("[INPUT LOCK] Locked by: %s (Count: %d)"), *LockSourceToString(Source), Count);
	if (!bWasLocked)
	{
		BroadcastLockStateChange();
	}
}
void UGridInputLockComponent::ReleaseLock(EInputLockSource Source)
{
	bool bWasLocked = IsLocked();
	if (int32* CountPtr = LockCounts.Find(Source))
	{
		if (*CountPtr > 0)
		{
			--(*CountPtr);
			UE_LOG(LogTemp, Log, TEXT("[INPUT LOCK] Released by: %s (Remaining: %d)"), *LockSourceToString(Source), *CountPtr);
			if (*CountPtr == 0)
			{
				LockCounts.Remove(Source);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[INPUT LOCK] Tried to release %s but count already 0"), *LockSourceToString(Source));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[INPUT LOCK] Tried to release non-existent lock: %s"), *LockSourceToString(Source));
	}
	if (bWasLocked && !IsLocked())
	{
		BroadcastLockStateChange();
	}
}
void UGridInputLockComponent::ForceUnlockAll()
{
	if (LockCounts.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[INPUT LOCK] Force unlocking all (%d sources)"), LockCounts.Num());
		LockCounts.Empty();
		BroadcastLockStateChange();
	}
}
bool UGridInputLockComponent::IsLocked() const
{
	for (const auto& Pair : LockCounts)
	{
		if (Pair.Value > 0)
		{
			return true;
		}
	}
	return false;
}
bool UGridInputLockComponent::IsLockedBy(EInputLockSource Source) const
{
	const int32* CountPtr = LockCounts.Find(Source);
	return CountPtr && *CountPtr > 0;
}
FString UGridInputLockComponent::GetLockDebugInfo() const
{
	if (!IsLocked())
	{
		return TEXT("Unlocked");
	}
	TArray<FString> ActiveLocks;
	for (const auto& Pair : LockCounts)
	{
		if (Pair.Value > 0)
		{
			ActiveLocks.Add(FString::Printf(TEXT("%s(%d)"), *LockSourceToString(Pair.Key), Pair.Value));
		}
	}
	return FString::Join(ActiveLocks, TEXT(", "));
}
void UGridInputLockComponent::BroadcastLockStateChange()
{
	bool bCurrentlyLocked = IsLocked();
	UE_LOG(LogTemp, Log, TEXT("[INPUT LOCK] State changed: %s"), bCurrentlyLocked ? TEXT("LOCKED") : TEXT("UNLOCKED"));
	OnInputLockChanged.Broadcast(bCurrentlyLocked);
}
FString UGridInputLockComponent::LockSourceToString(EInputLockSource Source) const
{
	switch (Source)
	{
	case EInputLockSource::TurnTransition: return TEXT("TurnTransition");
	case EInputLockSource::AbilityExecution: return TEXT("AbilityExecution");
	case EInputLockSource::PresentationPlaying: return TEXT("PresentationPlaying");
	case EInputLockSource::AIThinking: return TEXT("AIThinking");
	case EInputLockSource::MovementProcessing: return TEXT("MovementProcessing");
	default: return TEXT("Unknown");
	}
}

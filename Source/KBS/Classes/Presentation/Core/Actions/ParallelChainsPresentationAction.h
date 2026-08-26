#pragma once
#include "CoreMinimal.h"
#include "Presentation/Core/PresentationSequenceAction.h"
#include "ParallelChainsPresentationAction.generated.h"

// Abstract base for actions that run N independent chains concurrently and finish once all complete.
//
// Usage:
//  - Derived class OnExecute: call InitRemainingChains(N) once, then start each chain.
//  - When a chain completes: call NotifyChainFinished().
//  - FinishExecution(Completed) is called automatically when the last chain reports in.
//
// Per-chain start delays and cleanup are the responsibility of derived classes.
UCLASS(Abstract, BlueprintType, Blueprintable)
class KBS_API UParallelChainsPresentationAction : public UPresentationSequenceAction
{
	GENERATED_BODY()

protected:
	// Call once in OnExecute before starting any chain.
	// If Count == 0, FinishExecution(Completed) is called immediately.
	void InitRemainingChains(int32 Count);

	// Call once per chain when it finishes.
	void NotifyChainFinished();

	EPlaybackMode CurrentPlaybackMode = EPlaybackMode::Animated;

private:
	int32 RemainingChains = 0;
};

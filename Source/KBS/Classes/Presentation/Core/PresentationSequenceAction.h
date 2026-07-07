#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Presentation/Core/PresentationSequencePlayer.h"
#include "PresentationSequenceAction.generated.h"

UENUM(BlueprintType)
enum class EVisualActionResult : uint8
{
	Completed,
	Timeout,
	Interrupted,
	MontageFail,
};

UENUM(BlueprintType)
enum class EPresentationTransitionPolicy : uint8
{
	WaitForExit,      // sequencer advances after OnPresentationExit; cleanup runs in background
	WaitForCleanup,   // sequencer advances only after OnPresentationCleanupExit
	KeepForSequence,  // never gates advancement; player cleans up at sequence end
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPresentationExit,
	UPresentationSequenceAction*, Action, EVisualActionResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPresentationCleanupExit,
	UPresentationSequenceAction*, Action, EVisualActionResult, Result);

UCLASS(Abstract, BlueprintType, Blueprintable)
class KBS_API UPresentationSequenceAction : public UObject
{
	GENERATED_BODY()

public:
	// --- External entry points (called by UPresentationSequencePlayer) ---

	virtual void Execute(EPlaybackMode PlaybackMode)
	{
		CurrentPlaybackMode = PlaybackMode;
		if (TimeoutDuration > 0.f)
		{
			StartTimeout(TimeoutDuration);
		}
		OnExecute(PlaybackMode);
	}

	virtual void Cleanup()
	{
		CancelTimeout();
		OnCleanup();
	}

	// Must be stable before Execute — the sequence chooses its delegate bindings from this
	virtual EPresentationTransitionPolicy GetTransitionPolicy() const { return TransitionPolicy; }
	EVisualActionResult GetResult() const { return Result; }

	UPROPERTY(BlueprintAssignable, Category = "Presentation")
	FOnPresentationExit OnPresentationExit;

	UPROPERTY(BlueprintAssignable, Category = "Presentation")
	FOnPresentationCleanupExit OnPresentationCleanupExit;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Presentation")
	EPresentationTransitionPolicy TransitionPolicy = EPresentationTransitionPolicy::WaitForExit;

	UPROPERTY(EditDefaultsOnly, Category = "Presentation", meta = (ClampMin = "0.0"))
	float TimeoutDuration = 0.f;

	// --- Subclass hooks ---

	virtual void OnExecute(EPlaybackMode PlaybackMode) {}
	virtual void OnCleanup() {}

	// Call when execution phase is done
	void FinishExecution(EVisualActionResult InResult)
	{
		CancelTimeout();
		Result = InResult;
		OnPresentationExit.Broadcast(this, Result);
	}

	// Call when cleanup phase is done
	void FinishCleanup()
	{
		OnPresentationCleanupExit.Broadcast(this, Result);
	}

	// Timeout mechanism — subclass overrides to arm/disarm its own timer
	virtual void StartTimeout(float Duration) {}
	virtual void CancelTimeout() {}

	// Subclass timer callback should call this
	void HandleTimeout()
	{
		FinishExecution(EVisualActionResult::Timeout);
	}

private:
	EVisualActionResult Result = EVisualActionResult::Completed;
	EPlaybackMode CurrentPlaybackMode = EPlaybackMode::Animated;
};

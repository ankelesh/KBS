#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Presentation/Core/PresentationSequenceAction.h"
#include "PresentationSequence.generated.h"

class UPresentationSequence;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSequenceComplete,
	UPresentationSequence*, Sequence);

UCLASS(BlueprintType)
class KBS_API UPresentationSequence : public UObject
{
	GENERATED_BODY()

public:
	void Play(EPlaybackMode PlaybackMode);

	bool IsPlaying() const { return bPlaying; }

	UPROPERTY(BlueprintAssignable, Category = "Presentation")
	FOnSequenceComplete OnSequenceComplete;

	UPROPERTY()
	TArray<TObjectPtr<UPresentationSequenceAction>> Actions;

	// TODO: shared context accumulation — add when a concrete action needs it

private:
	UFUNCTION()
	void HandleActionExit(UPresentationSequenceAction* Action, EVisualActionResult Result);
	UFUNCTION()
	void HandleActionCleanupExit(UPresentationSequenceAction* Action, EVisualActionResult Result);
	UFUNCTION()
	void HandlePendingCleanupExit(UPresentationSequenceAction* Action, EVisualActionResult Result);

	void StartAction(int32 Index);
	void AdvanceFromCurrent();
	void BeginEndOfSequenceCleanup();
	void FinishSequence();

	EPlaybackMode CurrentPlaybackMode = EPlaybackMode::Animated;
	int32 CurrentIndex = INDEX_NONE;
	bool bPlaying = false;

	UPROPERTY()
	TArray<TObjectPtr<UPresentationSequenceAction>> PendingCleanupActions;
	int32 PendingCleanupCount = 0;
};

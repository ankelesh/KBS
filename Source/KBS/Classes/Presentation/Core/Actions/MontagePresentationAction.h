#pragma once
#include "CoreMinimal.h"
#include "Presentation/Core/PresentationSequenceAction.h"
#include "Presentation/Core/Actions/MontagePlaybackHelper.h"
#include "MontagePresentationAction.generated.h"

// Single-actor montage action. Thin adapter over UMontagePlaybackHelper, which owns the actual
// montage/SFX/VFX/timeout mechanics (shared with multi-target actions such as
// UAoEReactionPresentationAction).
UCLASS(BlueprintType, Blueprintable)
class KBS_API UMontagePresentationAction : public UPresentationSequenceAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	FMontageActionContext Context;

protected:
	virtual void OnExecute(EPlaybackMode PlaybackMode) override;
	virtual void OnCleanup() override;
	virtual void StartTimeout(float Duration) override;
	virtual void CancelTimeout() override;

private:
	void HandlePlaybackFinished(EVisualActionResult Result);
	void HandlePlaybackCleanupFinished();

	UMontagePlaybackHelper* EnsurePlayback();

	UPROPERTY()
	TObjectPtr<UMontagePlaybackHelper> Playback;
};

#include "Presentation/Core/Actions/MontagePresentationAction.h"

UMontagePlaybackHelper* UMontagePresentationAction::EnsurePlayback()
{
	if (!Playback)
	{
		Playback = NewObject<UMontagePlaybackHelper>(this);
	}
	return Playback;
}

void UMontagePresentationAction::OnExecute(EPlaybackMode PlaybackMode)
{
	UMontagePlaybackHelper* PlaybackHelper = EnsurePlayback();
	PlaybackHelper->OnFinished.BindUObject(this, &UMontagePresentationAction::HandlePlaybackFinished);
	PlaybackHelper->Play(Context, PlaybackMode);
}

void UMontagePresentationAction::OnCleanup()
{
	UMontagePlaybackHelper* PlaybackHelper = EnsurePlayback();
	PlaybackHelper->OnCleanupFinished.BindUObject(this, &UMontagePresentationAction::HandlePlaybackCleanupFinished);
	PlaybackHelper->Cleanup();
}

void UMontagePresentationAction::StartTimeout(float Duration)
{
	UWorld* World = Context.Actor ? Context.Actor->GetWorld() : nullptr;
	EnsurePlayback()->StartTimeout(World, Duration);
}

void UMontagePresentationAction::CancelTimeout()
{
	if (Playback)
	{
		Playback->CancelTimeout();
	}
}

void UMontagePresentationAction::HandlePlaybackFinished(EVisualActionResult res)
{
	FinishExecution(res);
}

void UMontagePresentationAction::HandlePlaybackCleanupFinished()
{
	FinishCleanup();
}

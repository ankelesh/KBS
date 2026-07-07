#include "Presentation/Core/PresentationSequence.h"

DEFINE_LOG_CATEGORY_STATIC(LogKBSPresentation, Log, All);

void UPresentationSequence::Play(EPlaybackMode PlaybackMode)
{
	checkf(!bPlaying, TEXT("UPresentationSequence::Play called while already playing"));
	bPlaying = true;
	CurrentPlaybackMode = PlaybackMode;

	if (Actions.IsEmpty())
	{
		FinishSequence();
		return;
	}
	StartAction(0);
}

void UPresentationSequence::StartAction(int32 Index)
{
	// Consecutive KeepForSequence actions are launched in a loop — they never gate advancement
	while (Actions.IsValidIndex(Index))
	{
		CurrentIndex = Index;
		UPresentationSequenceAction* Action = Actions[Index];

		switch (Action->GetTransitionPolicy())
		{
		case EPresentationTransitionPolicy::WaitForExit:
			Action->OnPresentationExit.AddDynamic(this, &UPresentationSequence::HandleActionExit);
			Action->Execute(CurrentPlaybackMode);
			return;

		case EPresentationTransitionPolicy::WaitForCleanup:
			Action->OnPresentationExit.AddDynamic(this, &UPresentationSequence::HandleActionExit);
			Action->OnPresentationCleanupExit.AddDynamic(this, &UPresentationSequence::HandleActionCleanupExit);
			Action->Execute(CurrentPlaybackMode);
			return;

		case EPresentationTransitionPolicy::KeepForSequence:
			PendingCleanupActions.Add(Action);
			Action->Execute(CurrentPlaybackMode);
			++Index;
			break;
		}
	}
	BeginEndOfSequenceCleanup();
}

void UPresentationSequence::AdvanceFromCurrent()
{
	StartAction(CurrentIndex + 1);
}

void UPresentationSequence::HandleActionExit(UPresentationSequenceAction* Action, EVisualActionResult Result)
{
	if (Result != EVisualActionResult::Completed)
	{
		UE_LOG(LogKBSPresentation, Warning, TEXT("Action %s exited with result %s; sequence continues"),
			*Action->GetName(), *UEnum::GetValueAsString(Result));
	}

	Action->OnPresentationExit.RemoveDynamic(this, &UPresentationSequence::HandleActionExit);

	if (Action->GetTransitionPolicy() == EPresentationTransitionPolicy::WaitForCleanup)
	{
		// Advancement happens in HandleActionCleanupExit
		Action->Cleanup();
		return;
	}

	// WaitForExit: cleanup runs in background, advance now
	Action->Cleanup();
	AdvanceFromCurrent();
}

void UPresentationSequence::HandleActionCleanupExit(UPresentationSequenceAction* Action, EVisualActionResult Result)
{
	Action->OnPresentationCleanupExit.RemoveDynamic(this, &UPresentationSequence::HandleActionCleanupExit);
	AdvanceFromCurrent();
}

void UPresentationSequence::BeginEndOfSequenceCleanup()
{
	if (PendingCleanupActions.IsEmpty())
	{
		FinishSequence();
		return;
	}

	PendingCleanupCount = PendingCleanupActions.Num();

	// Bind all before any Cleanup call — cleanup may broadcast synchronously
	for (UPresentationSequenceAction* Action : PendingCleanupActions)
	{
		Action->OnPresentationCleanupExit.AddDynamic(this, &UPresentationSequence::HandlePendingCleanupExit);
	}
	for (int32 i = 0; i < PendingCleanupActions.Num(); ++i)
	{
		PendingCleanupActions[i]->Cleanup();
	}
}

void UPresentationSequence::HandlePendingCleanupExit(UPresentationSequenceAction* Action, EVisualActionResult Result)
{
	Action->OnPresentationCleanupExit.RemoveDynamic(this, &UPresentationSequence::HandlePendingCleanupExit);
	--PendingCleanupCount;
	if (PendingCleanupCount == 0)
	{
		FinishSequence();
	}
}

void UPresentationSequence::FinishSequence()
{
	bPlaying = false;
	CurrentIndex = INDEX_NONE;
	PendingCleanupActions.Empty();
	PendingCleanupCount = 0;
	OnSequenceComplete.Broadcast(this);
}

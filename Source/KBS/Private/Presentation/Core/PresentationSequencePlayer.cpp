#include "Presentation/Core/PresentationSequencePlayer.h"
#include "Presentation/Core/PresentationSequence.h"

UPresentationSequencePlayer* UPresentationSequencePlayer::Get(const UObject* WorldContextObject)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetSubsystem<UPresentationSequencePlayer>();
	}
	return nullptr;
}

void UPresentationSequencePlayer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPresentationSequencePlayer::Deinitialize()
{
	Queue.Empty();
	QueueIndex = 0;
	State = EPresentationPlayerState::Idle;
	Super::Deinitialize();
}

void UPresentationSequencePlayer::EnqueueSequence(UPresentationSequence* Sequence)
{
	checkf(State == EPresentationPlayerState::Idle, TEXT("EnqueueSequence called while playing"));
	checkf(Sequence, TEXT("EnqueueSequence received null sequence"));
	Queue.Add(Sequence);
}

void UPresentationSequencePlayer::Play(EPlaybackMode PlaybackMode)
{
	checkf(State == EPresentationPlayerState::Idle, TEXT("Play called while already playing"));
	State = EPresentationPlayerState::Playing;
	CurrentPlaybackMode = PlaybackMode;
	QueueIndex = 0;
	PlayNext();
}

void UPresentationSequencePlayer::PlayNext()
{
	if (!Queue.IsValidIndex(QueueIndex))
	{
		Queue.Empty();
		QueueIndex = 0;
		State = EPresentationPlayerState::Idle;
		OnPresentationComplete.Broadcast();
		return;
	}

	UPresentationSequence* Sequence = Queue[QueueIndex];
	Sequence->OnSequenceComplete.AddDynamic(this, &UPresentationSequencePlayer::HandleSequenceComplete);
	Sequence->Play(CurrentPlaybackMode);
}

void UPresentationSequencePlayer::HandleSequenceComplete(UPresentationSequence* Sequence)
{
	Sequence->OnSequenceComplete.RemoveDynamic(this, &UPresentationSequencePlayer::HandleSequenceComplete);
	++QueueIndex;
	PlayNext();
}

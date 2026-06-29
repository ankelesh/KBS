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
	Super::Deinitialize();
}

void UPresentationSequencePlayer::PlaySequence(UPresentationSequence* Sequence, EPlaybackMode PlaybackMode)
{
}

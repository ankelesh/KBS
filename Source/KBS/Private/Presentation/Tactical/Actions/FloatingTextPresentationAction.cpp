#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"
#include "UI/Common/Widgets/FloatingTextActor.h"

namespace
{
	constexpr float FloatingTextSpawnHeight = 150.f;
}

void UFloatingTextPresentationAction::OnExecute(EPlaybackMode PlaybackMode)
{
	checkf(Context.Actor, TEXT("UFloatingTextPresentationAction: Actor must not be null"));

	UWorld* World = Context.Actor->GetWorld();
	const FVector SpawnLocation = Context.Actor->GetActorLocation() + FVector(0.f, 0.f, FloatingTextSpawnHeight);

	AFloatingTextActor* TextActor = World->SpawnActor<AFloatingTextActor>(SpawnLocation, FRotator::ZeroRotator);
	if (TextActor)
	{
		TextActor->Init(Context.Text, Context.Color);
	}

	FinishExecution(EVisualActionResult::Completed);
}

void UFloatingTextPresentationAction::OnCleanup()
{
	FinishCleanup();
}

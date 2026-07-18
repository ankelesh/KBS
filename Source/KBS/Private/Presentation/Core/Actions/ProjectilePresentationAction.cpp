#include "Presentation/Core/Actions/ProjectilePresentationAction.h"

void UProjectilePresentationAction::OnExecute(EPlaybackMode PlaybackMode)
{
	checkf(Context.WorldContextActor, TEXT("UProjectilePresentationAction: WorldContextActor must not be null"));

	UClass* ActorClass = Context.ActorClass ? Context.ActorClass.Get() : AProjectilePresentationActor::StaticClass();
	SpawnedProjectile = Context.WorldContextActor->GetWorld()->SpawnActor<AProjectilePresentationActor>(
		ActorClass, Context.StartLocation, FRotator::ZeroRotator);

	SpawnedProjectile->OnFinished.BindUObject(this, &UProjectilePresentationAction::HandleProjectileFinished);
	SpawnedProjectile->Launch(Context.StartLocation, Context.EndLocation, Context.Speed,
		Context.TrailVFX, Context.TrailSFX, Context.ImpactVFX, Context.ImpactSFX);
}

void UProjectilePresentationAction::HandleProjectileFinished()
{
	FinishExecution(EVisualActionResult::Completed);
}

void UProjectilePresentationAction::OnCleanup()
{
	if (SpawnedProjectile)
	{
		SpawnedProjectile->Destroy();
		SpawnedProjectile = nullptr;
	}

	FinishCleanup();
}

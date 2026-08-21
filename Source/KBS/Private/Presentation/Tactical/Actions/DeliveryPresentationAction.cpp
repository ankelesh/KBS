#include "Presentation/Tactical/Actions/DeliveryPresentationAction.h"
#include "Presentation/Core/Actions/ProjectilePresentationActor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

void UDeliveryPresentationAction::OnExecute(EPlaybackMode PlaybackMode)
{
	CurrentPlaybackMode = PlaybackMode;

	const int32 ProjCount = ProjectileContexts.Num();
	const int32 VfxCount  = VfxChains.Num();
	const int32 Total     = ProjCount + VfxCount;

	InitRemainingChains(Total);
	if (Total == 0)
	{
		return;
	}

	StartDelayTimers.SetNum(Total);

	auto LaunchChain = [this, ProjCount](int32 ChainIndex)
	{
		if (ChainIndex < ProjCount)
		{
			ExecuteProjectileChain(ChainIndex, CurrentPlaybackMode);
		}
		else
		{
			ExecuteVfxChain(ChainIndex - ProjCount);
		}
	};

	for (int32 i = 0; i < Total; ++i)
	{
		const float Delay = ChainStartDelays.IsValidIndex(i) ? ChainStartDelays[i] : 0.f;

		if (Delay <= 0.f || PlaybackMode == EPlaybackMode::Instant)
		{
			LaunchChain(i);
		}
		else
		{
			// World from projectile or VFX context actor; one must be valid when delays are used.
			UWorld* World = nullptr;
			if (ProjectileContexts.IsValidIndex(i) && ProjectileContexts[i].WorldContextActor)
				World = ProjectileContexts[i].WorldContextActor->GetWorld();
			else if (!ProjectileContexts.IsEmpty() && ProjectileContexts[0].WorldContextActor)
				World = ProjectileContexts[0].WorldContextActor->GetWorld();
			else if (!VfxChains.IsEmpty() && VfxChains[0].WorldContextActor)
				World = VfxChains[0].WorldContextActor->GetWorld();

			checkf(World, TEXT("UDeliveryPresentationAction: cannot resolve world for chain start delay timer"));

			World->GetTimerManager().SetTimer(
				StartDelayTimers[i],
				FTimerDelegate::CreateLambda([this, i, ProjCount]()
				{
					if (i < ProjCount)
						ExecuteProjectileChain(i, CurrentPlaybackMode);
					else
						ExecuteVfxChain(i - ProjCount);
				}),
				Delay, /*bLoop*/ false);
		}
	}
}

void UDeliveryPresentationAction::ExecuteProjectileChain(int32 ChainIndex, EPlaybackMode PlaybackMode)
{
	const FProjectileActionContext& Ctx = ProjectileContexts[ChainIndex];

	if (PlaybackMode == EPlaybackMode::Instant)
	{
		NotifyChainFinished();
		return;
	}

	UClass* ActorClass = Ctx.ActorClass ? Ctx.ActorClass.Get() : AProjectilePresentationActor::StaticClass();
	AProjectilePresentationActor* Projectile = Ctx.WorldContextActor->GetWorld()->SpawnActor<AProjectilePresentationActor>(
		ActorClass, Ctx.StartLocation, FRotator::ZeroRotator);

	SpawnedProjectiles.Add(Projectile);

	Projectile->OnFinished.BindLambda([this]()
	{
		NotifyChainFinished();
	});

	Projectile->Launch(Ctx.StartLocation, Ctx.EndLocation, Ctx.Speed,
		Ctx.TrailVFX, Ctx.TrailSFX, Ctx.ImpactVFX, Ctx.ImpactSFX);
}

void UDeliveryPresentationAction::ExecuteVfxChain(int32 VfxIndex)
{
	const FDeliveryVfxChain& Chain = VfxChains[VfxIndex];
	if (Chain.ImpactVFX && Chain.WorldContextActor && CurrentPlaybackMode == EPlaybackMode::Animated)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			Chain.WorldContextActor->GetWorld(),
			Chain.ImpactVFX,
			Chain.Location,
			FRotator::ZeroRotator, FVector::OneVector,
			/*bAutoDestroy*/ true, /*bAutoActivate*/ true,
			ENCPoolMethod::None);
	}
	NotifyChainFinished();
}

void UDeliveryPresentationAction::OnCleanup()
{
	// Cancel any pending start-delay timers. Locate world from a live context actor.
	UWorld* TimerWorld = nullptr;
	if (!SpawnedProjectiles.IsEmpty() && SpawnedProjectiles[0])
	{
		TimerWorld = SpawnedProjectiles[0]->GetWorld();
	}
	else if (!ProjectileContexts.IsEmpty() && ProjectileContexts[0].WorldContextActor)
	{
		TimerWorld = ProjectileContexts[0].WorldContextActor->GetWorld();
	}
	else if (!VfxChains.IsEmpty() && VfxChains[0].WorldContextActor)
	{
		TimerWorld = VfxChains[0].WorldContextActor->GetWorld();
	}
	if (TimerWorld)
	{
		for (FTimerHandle& Handle : StartDelayTimers)
		{
			TimerWorld->GetTimerManager().ClearTimer(Handle);
		}
	}

	for (AProjectilePresentationActor* Projectile : SpawnedProjectiles)
	{
		if (Projectile)
		{
			Projectile->Destroy();
		}
	}
	SpawnedProjectiles.Empty();

	FinishCleanup();
}

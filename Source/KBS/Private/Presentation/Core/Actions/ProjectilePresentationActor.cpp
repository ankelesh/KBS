#include "Presentation/Core/Actions/ProjectilePresentationActor.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AProjectilePresentationActor::AProjectilePresentationActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

void AProjectilePresentationActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void AProjectilePresentationActor::Launch(const FVector& Start, const FVector& End, float Speed,
                                           UNiagaraSystem* TrailVFX, USoundBase* TrailSFX,
                                           UNiagaraSystem* ImpactVFX, USoundBase* ImpactSFX)
{
	StartLocation = Start;
	EndLocation = End;
	Duration = FVector::Dist(Start, End) / FMath::Max(Speed, 1.f);
	Elapsed = 0.f;
	PendingImpactVFX = ImpactVFX;
	PendingImpactSFX = ImpactSFX;

	SetActorLocation(StartLocation);

	if (TrailVFX)
	{
		TrailVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailVFX, Root, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget, true);
	}

	if (TrailSFX)
	{
		TrailSFXComponent = UGameplayStatics::SpawnSoundAttached(TrailSFX, Root);
	}

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &AProjectilePresentationActor::OnTick), 0.f);
}

bool AProjectilePresentationActor::OnTick(float DeltaTime)
{
	Elapsed += DeltaTime;
	const float t = FMath::Clamp(Duration > 0.f ? Elapsed / Duration : 1.f, 0.f, 1.f);

	SetActorLocation(FMath::Lerp(StartLocation, EndLocation, t));

	if (t >= 1.f)
	{
		TickerHandle.Reset();

		if (TrailVFXComponent)
		{
			TrailVFXComponent->DeactivateImmediate();
			TrailVFXComponent = nullptr;
		}
		if (TrailSFXComponent)
		{
			TrailSFXComponent->Stop();
			TrailSFXComponent = nullptr;
		}

		PlayImpact();
		return false;
	}

	return true;
}

void AProjectilePresentationActor::PlayImpact()
{
	if (PendingImpactSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PendingImpactSFX, EndLocation);
	}

	if (!PendingImpactVFX)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(), PendingImpactVFX, EndLocation, FRotator::ZeroRotator, FVector::OneVector,
		/*bAutoDestroy*/ false, /*bAutoActivate*/ true, ENCPoolMethod::None, /*bPreCullCheck*/ true);
	if (!Comp)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	Comp->OnSystemFinished.AddDynamic(this, &AProjectilePresentationActor::HandleImpactVFXFinished);
}

void AProjectilePresentationActor::HandleImpactVFXFinished(UNiagaraComponent* FinishedComponent)
{
	OnFinished.ExecuteIfBound();
}

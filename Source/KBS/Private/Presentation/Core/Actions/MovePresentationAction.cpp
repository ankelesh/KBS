#include "Presentation/Core/Actions/MovePresentationAction.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

void UMovePresentationAction::OnExecute(EPlaybackMode PlaybackMode)
{
	checkf(Context.Actor, TEXT("UMovePresentationAction: Actor must not be null"));
	checkf(Context.Path.Num() > 0, TEXT("UMovePresentationAction: Path must have at least one segment"));

	if (PlaybackMode == EPlaybackMode::Instant)
	{
		ApplyFinalTransform();
		FinishExecution(EVisualActionResult::Completed);
		return;
	}

	CurrentSegmentIndex = 0;
	SegmentProgress = 0.f;

	StartSFX();
	StartVFX();

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UMovePresentationAction::OnTick), 0.f);
}

bool UMovePresentationAction::OnTick(float DeltaTime)
{
	if (!Context.Actor || CurrentSegmentIndex >= Context.Path.Num())
	{
		ApplyFinalTransform();
		FinishExecution(EVisualActionResult::Completed);
		return false;
	}

	SegmentProgress += DeltaTime;
	const FPresentationMoveSegment& Seg = Context.Path[CurrentSegmentIndex];

	const float t = (Seg.Duration > 0.f)
		? FMath::Clamp(SegmentProgress / Seg.Duration, 0.f, 1.f)
		: 1.f;

	Context.Actor->SetActorLocation(FMath::Lerp(Seg.Start, Seg.End, t));
	Context.Actor->SetActorRotation(
		FMath::RInterpTo(Context.Actor->GetActorRotation(), Seg.TargetRotation, DeltaTime, 360.f));

	if (t >= 1.f)
	{
		SegmentProgress -= Seg.Duration;
		++CurrentSegmentIndex;

		if (CurrentSegmentIndex >= Context.Path.Num())
		{
			ApplyFinalTransform();
			FinishExecution(EVisualActionResult::Completed);
			return false;
		}
	}

	return true;
}

void UMovePresentationAction::OnCleanup()
{
	StopTicker();
	// Idempotent; covers KeepForSequence cleanup arriving mid-move
	ApplyFinalTransform();
	StopVFX();
	StopSFX();
	FinishCleanup();
}

void UMovePresentationAction::StopTicker()
{
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}
}

void UMovePresentationAction::ApplyFinalTransform()
{
	if (!Context.Actor || Context.Path.Num() == 0)
	{
		return;
	}
	const FPresentationMoveSegment& Last = Context.Path.Last();
	Context.Actor->SetActorLocationAndRotation(Last.End, Last.TargetRotation);
}

void UMovePresentationAction::StartSFX()
{
	if (!Context.SFX)
	{
		return;
	}
	ActiveSFX = UGameplayStatics::SpawnSoundAttached(Context.SFX, Context.Actor->GetRootComponent());
}

void UMovePresentationAction::StopSFX()
{
	if (ActiveSFX)
	{
		ActiveSFX->Stop();
		ActiveSFX = nullptr;
	}
}

void UMovePresentationAction::StartVFX()
{
	for (UNiagaraSystem* System : Context.VFX)
	{
		if (!System)
		{
			continue;
		}
		UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			System,
			Context.Actor->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true);
		if (Comp)
		{
			ActiveVFX.Add(Comp);
		}
	}
}

void UMovePresentationAction::StopVFX()
{
	for (UNiagaraComponent* Comp : ActiveVFX)
	{
		if (Comp)
		{
			Comp->DeactivateImmediate();
		}
	}
	ActiveVFX.Reset();
}

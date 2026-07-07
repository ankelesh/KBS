#include "Presentation/Core/Actions/MontagePresentationAction.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNotifyQueue.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

UAnimInstance* UMontagePresentationAction::GetAnimInstance() const
{
	USkeletalMeshComponent* Mesh = Context.Actor->FindComponentByClass<USkeletalMeshComponent>();
	return Mesh ? Mesh->GetAnimInstance() : nullptr;
}

void UMontagePresentationAction::OnExecute(EPlaybackMode PlaybackMode)
{
	checkf(Context.Actor, TEXT("UMontagePresentationAction: Actor must not be null"));
	checkf(Context.Montage, TEXT("UMontagePresentationAction: Montage must not be null"));

	if (PlaybackMode == EPlaybackMode::Instant)
	{
		FinishExecution(EVisualActionResult::Completed);
		return;
	}

	UAnimInstance* AnimInstance = GetAnimInstance();
	checkf(AnimInstance, TEXT("UMontagePresentationAction: Actor has no AnimInstance"));

	bExitedViaSignal = false;

	StartSFX();
	StartVFX();

	AnimInstance->OnMontageEnded.AddUObject(this, &UMontagePresentationAction::OnMontageEnded);

	if (!Context.EarlyExitNotifyName.IsNone())
	{
		AnimInstance->OnPlayMontageNotifyBegin.AddUObject(this, &UMontagePresentationAction::OnMontageNotifyBegin);
	}

	if (AnimInstance->Montage_Play(Context.Montage, 1.f) == 0.f)
	{
		FinishExecution(EVisualActionResult::MontageFail);
	}
}

void UMontagePresentationAction::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	if (NotifyName != Context.EarlyExitNotifyName)
	{
		return;
	}

	UAnimInstance* AI = GetAnimInstance();
	if (AI)
	{
		AI->OnPlayMontageNotifyBegin.RemoveAll(this);
		// Natural end is only observed again if OnCleanup rebinds OnMontageEndedCleanup
		AI->OnMontageEnded.RemoveAll(this);
	}

	bExitedViaSignal = true;
	FinishExecution(EVisualActionResult::Completed);
}

void UMontagePresentationAction::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != Context.Montage)
	{
		return;
	}

	UAnimInstance* AI = GetAnimInstance();
	if (AI)
	{
		AI->OnMontageEnded.RemoveAll(this);
		AI->OnPlayMontageNotifyBegin.RemoveAll(this);
	}

	FinishExecution(bInterrupted ? EVisualActionResult::Interrupted : EVisualActionResult::Completed);
}

void UMontagePresentationAction::OnMontageEndedCleanup(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != Context.Montage)
	{
		return;
	}

	UAnimInstance* AI = GetAnimInstance();
	if (AI)
	{
		AI->OnMontageEnded.RemoveAll(this);
	}

	FinishCleanup();
}

void UMontagePresentationAction::OnCleanup()
{
	StopSFX();
	StopVFX();

	if (bExitedViaSignal)
	{
		UAnimInstance* AI = GetAnimInstance();
		if (AI && AI->Montage_IsPlaying(Context.Montage))
		{
			AI->OnMontageEnded.AddUObject(this, &UMontagePresentationAction::OnMontageEndedCleanup);
		}
		else
		{
			// Montage already finished (KeepForSequence: cleanup arrives at sequence end)
			FinishCleanup();
		}
	}
	else
	{
		StopMontage();
		FinishCleanup();
	}
}

void UMontagePresentationAction::StartTimeout(float Duration)
{
	UWorld* World = Context.Actor ? Context.Actor->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}
	World->GetTimerManager().SetTimer(TimeoutHandle, this, &UMontagePresentationAction::HandleTimeout, Duration, false);
}

void UMontagePresentationAction::CancelTimeout()
{
	UWorld* World = Context.Actor ? Context.Actor->GetWorld() : nullptr;
	if (World && TimeoutHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(TimeoutHandle);
	}
}

void UMontagePresentationAction::StopMontage()
{
	if (!Context.Actor || !Context.Montage)
	{
		return;
	}
	UAnimInstance* AI = GetAnimInstance();
	if (!AI)
	{
		return;
	}
	AI->OnMontageEnded.RemoveAll(this);
	AI->OnPlayMontageNotifyBegin.RemoveAll(this);
	if (AI->Montage_IsPlaying(Context.Montage))
	{
		AI->Montage_Stop(0.f, Context.Montage);
	}
}

void UMontagePresentationAction::StartSFX()
{
	if (!Context.SFX)
	{
		return;
	}
	ActiveSFX = UGameplayStatics::SpawnSoundAttached(Context.SFX, Context.Actor->GetRootComponent());
}

void UMontagePresentationAction::StopSFX()
{
	if (ActiveSFX)
	{
		ActiveSFX->Stop();
		ActiveSFX = nullptr;
	}
}

void UMontagePresentationAction::StartVFX()
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

void UMontagePresentationAction::StopVFX()
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

#include "Presentation/Core/Actions/MontagePlaybackHelper.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifyQueue.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogMontagePlayback, Log, All);

UAnimInstance* UMontagePlaybackHelper::GetAnimInstance() const
{
	USkeletalMeshComponent* Mesh = Context.Actor->FindComponentByClass<USkeletalMeshComponent>();
	return Mesh ? Mesh->GetAnimInstance() : nullptr;
}

void UMontagePlaybackHelper::Play(const FMontageActionContext& InContext, EPlaybackMode PlaybackMode)
{
	checkf(InContext.Actor, TEXT("UMontagePlaybackHelper: Actor must not be null"));
	checkf(InContext.Montage, TEXT("UMontagePlaybackHelper: Montage must not be null"));

	Context = InContext;

	if (PlaybackMode == EPlaybackMode::Instant)
	{
		CancelTimeout();
		OnBlendingOut.ExecuteIfBound(false);
		OnFinished.ExecuteIfBound(EVisualActionResult::Completed);
		return;
	}

	UAnimInstance* AnimInstance = GetAnimInstance();
	checkf(AnimInstance, TEXT("UMontagePlaybackHelper: Actor has no AnimInstance"));

	bExitedViaSignal = false;

	StartSFX();
	StartVFX();

	AnimInstance->OnMontageEnded.AddDynamic(this, &UMontagePlaybackHelper::OnMontageEnded);
	AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UMontagePlaybackHelper::OnMontageBlendingOut);

	if (!Context.EarlyExitNotifyName.IsNone())
	{
		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UMontagePlaybackHelper::OnMontageNotifyBegin);
	}

	const float PlayedLength = AnimInstance->Montage_Play(Context.Montage, 1.f);
	UE_LOG(LogMontagePlayback, Log, TEXT("[%s] Play %s on %s: Montage_Play returned %.3f (asset length %.3f)"),
		*GetName(), *Context.Montage->GetName(), *Context.Actor->GetName(), PlayedLength, Context.Montage->GetPlayLength());

	if (PlayedLength == 0.f)
	{
		CancelTimeout();
		OnFinished.ExecuteIfBound(EVisualActionResult::MontageFail);
	}
}

void UMontagePlaybackHelper::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	if (NotifyName != Context.EarlyExitNotifyName)
	{
		return;
	}

	UE_LOG(LogMontagePlayback, Log, TEXT("[%s] EarlyExit notify '%s' hit on %s (montage %s keeps playing)"),
		*GetName(), *NotifyName.ToString(), *Context.Actor->GetName(), *Context.Montage->GetName());

	UAnimInstance* AI = GetAnimInstance();
	if (AI)
	{
		AI->OnPlayMontageNotifyBegin.RemoveAll(this);
		// Natural end is only observed again if Cleanup rebinds OnMontageEndedCleanup
		AI->OnMontageEnded.RemoveAll(this);
		AI->OnMontageBlendingOut.RemoveAll(this);
	}

	CancelTimeout();
	bExitedViaSignal = true;
	// Montage keeps playing past this point, but we've already detached from its delegates above,
	// so give blend-out listeners their synthetic firing now rather than never.
	OnBlendingOut.ExecuteIfBound(false);
	OnFinished.ExecuteIfBound(EVisualActionResult::Completed);
}

void UMontagePlaybackHelper::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != Context.Montage)
	{
		UE_LOG(LogMontagePlayback, Verbose, TEXT("[%s] OnMontageEnded for unrelated montage %s (expected %s) - ignored"),
			*GetName(), *Montage->GetName(), *Context.Montage->GetName());
		return;
	}

	UE_LOG(LogMontagePlayback, Log, TEXT("[%s] OnMontageEnded %s on %s: bInterrupted=%s"),
		*GetName(), *Montage->GetName(), *Context.Actor->GetName(), bInterrupted ? TEXT("true") : TEXT("false"));

	CancelTimeout();

	UAnimInstance* AI = GetAnimInstance();
	if (AI)
	{
		AI->OnMontageEnded.RemoveAll(this);
		AI->OnPlayMontageNotifyBegin.RemoveAll(this);
		AI->OnMontageBlendingOut.RemoveAll(this);
	}

	OnFinished.ExecuteIfBound(bInterrupted ? EVisualActionResult::Interrupted : EVisualActionResult::Completed);
}

void UMontagePlaybackHelper::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != Context.Montage)
	{
		return;
	}

	UE_LOG(LogMontagePlayback, Log, TEXT("[%s] OnMontageBlendingOut %s on %s: bInterrupted=%s"),
		*GetName(), *Montage->GetName(), *Context.Actor->GetName(), bInterrupted ? TEXT("true") : TEXT("false"));

	UAnimInstance* AI = GetAnimInstance();
	if (AI)
	{
		AI->OnMontageBlendingOut.RemoveAll(this);
	}

	OnBlendingOut.ExecuteIfBound(bInterrupted);
}

void UMontagePlaybackHelper::OnMontageEndedCleanup(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != Context.Montage)
	{
		return;
	}

	UE_LOG(LogMontagePlayback, Log, TEXT("[%s] OnMontageEndedCleanup %s on %s: bInterrupted=%s"),
		*GetName(), *Montage->GetName(), *Context.Actor->GetName(), bInterrupted ? TEXT("true") : TEXT("false"));

	UAnimInstance* AI = GetAnimInstance();
	if (AI)
	{
		AI->OnMontageEnded.RemoveAll(this);
	}

	OnCleanupFinished.ExecuteIfBound();
}

void UMontagePlaybackHelper::Cleanup()
{
	StopSFX();
	StopVFX();

	UE_LOG(LogMontagePlayback, Log, TEXT("[%s] Cleanup on %s: bExitedViaSignal=%s"),
		*GetName(), Context.Actor ? *Context.Actor->GetName() : TEXT("null"), bExitedViaSignal ? TEXT("true") : TEXT("false"));

	if (bExitedViaSignal)
	{
		UAnimInstance* AI = GetAnimInstance();
		if (AI && AI->Montage_IsPlaying(Context.Montage))
		{
			AI->OnMontageEnded.AddDynamic(this, &UMontagePlaybackHelper::OnMontageEndedCleanup);
		}
		else
		{
			// Montage already finished on its own
			OnCleanupFinished.ExecuteIfBound();
		}
	}
	else
	{
		StopMontage();
		OnCleanupFinished.ExecuteIfBound();
	}
}

void UMontagePlaybackHelper::StartTimeout(UWorld* World, float Duration)
{
	if (!World)
	{
		return;
	}
	World->GetTimerManager().SetTimer(TimeoutHandle, this, &UMontagePlaybackHelper::HandleTimeout, Duration, false);
}

void UMontagePlaybackHelper::CancelTimeout()
{
	UWorld* World = Context.Actor ? Context.Actor->GetWorld() : nullptr;
	if (World && TimeoutHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(TimeoutHandle);
	}
}

void UMontagePlaybackHelper::HandleTimeout()
{
	UE_LOG(LogMontagePlayback, Warning, TEXT("[%s] Timeout fired on %s (montage %s) - bExitedViaSignal=%s"),
		*GetName(), Context.Actor ? *Context.Actor->GetName() : TEXT("null"),
		Context.Montage ? *Context.Montage->GetName() : TEXT("null"),
		bExitedViaSignal ? TEXT("true") : TEXT("false"));
	// Pose-hold montages (the main reason a timeout is used) never blend out on their own, so give
	// blend-out listeners a chance here too - otherwise a timeout-driven death step never flips.
	OnBlendingOut.ExecuteIfBound(false);
	OnFinished.ExecuteIfBound(EVisualActionResult::Timeout);
}

void UMontagePlaybackHelper::StopMontage()
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
	AI->OnMontageBlendingOut.RemoveAll(this);
	if (AI->Montage_IsPlaying(Context.Montage))
	{
		UE_LOG(LogMontagePlayback, Warning, TEXT("[%s] StopMontage hard-stopping still-playing %s on %s"),
			*GetName(), *Context.Montage->GetName(), *Context.Actor->GetName());
		AI->Montage_Stop(0.f, Context.Montage);
	}
}

void UMontagePlaybackHelper::StartSFX()
{
	if (!Context.SFX)
	{
		return;
	}
	ActiveSFX = UGameplayStatics::SpawnSoundAttached(Context.SFX, Context.Actor->GetRootComponent());
}

void UMontagePlaybackHelper::StopSFX()
{
	if (ActiveSFX)
	{
		ActiveSFX->Stop();
		ActiveSFX = nullptr;
	}
}

void UMontagePlaybackHelper::StartVFX()
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

void UMontagePlaybackHelper::StopVFX()
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

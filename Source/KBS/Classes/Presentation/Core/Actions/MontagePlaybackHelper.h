#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Presentation/Core/PresentationSequencePlayer.h"
#include "Presentation/Core/PresentationSequenceAction.h"
#include "MontagePlaybackHelper.generated.h"

class USoundBase;
class UAudioComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UAnimMontage;
class UAnimInstance;
struct FBranchingPointNotifyPayload;

USTRUCT(BlueprintType)
struct FMontageActionContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	TObjectPtr<UAnimMontage> Montage;

	// If set, fires OnFinished when this notify name is hit; montage keeps playing until natural end
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	FName EarlyExitNotifyName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Effects")
	TObjectPtr<USoundBase> SFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Effects")
	TArray<TObjectPtr<UNiagaraSystem>> VFX;
};

DECLARE_DELEGATE_OneParam(FOnMontagePlaybackFinished, EVisualActionResult);
DECLARE_DELEGATE(FOnMontagePlaybackCleanupFinished);
// Fires when the montage's blend-out starts, ahead of OnFinished - use for state changes that must
// land before the blend visually reads as "ending" (e.g. death pose swaps), since OnFinished/OnMontageEnded
// fires only after blend-out completes and looks like a flicker if a death pose swap waits for it.
DECLARE_DELEGATE_OneParam(FOnMontagePlaybackBlendingOut, bool /*bInterrupted*/);

// Plays a single montage on a single actor and reports back via plain (non-dynamic) delegates.
// Deliberately NOT a UPresentationSequenceAction - no TransitionPolicy, no player-facing exit
// contract. This is the shared leaf-level playback engine reused by UMontagePresentationAction
// (one actor) and multi-target actions like UAoEReactionPresentationAction (many actors at once),
// so the montage/SFX/VFX/timeout mechanics only exist in one place.
UCLASS()
class KBS_API UMontagePlaybackHelper : public UObject
{
	GENERATED_BODY()

public:
	FOnMontagePlaybackFinished OnFinished;
	FOnMontagePlaybackCleanupFinished OnCleanupFinished;
	FOnMontagePlaybackBlendingOut OnBlendingOut;

	void Play(const FMontageActionContext& InContext, EPlaybackMode PlaybackMode);

	// World is taken explicitly - StartTimeout can be called before Play() has populated Context.
	void StartTimeout(UWorld* World, float Duration);
	void CancelTimeout();
	void Cleanup();

private:
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnMontageEndedCleanup(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);
	UFUNCTION()
	void HandleTimeout();

	void StartSFX();
	void StopSFX();
	void StartVFX();
	void StopVFX();
	void StopMontage();
	UAnimInstance* GetAnimInstance() const;

	UPROPERTY()
	FMontageActionContext Context;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ActiveSFX;

	UPROPERTY()
	TArray<TObjectPtr<UNiagaraComponent>> ActiveVFX;

	FTimerHandle TimeoutHandle;
	bool bExitedViaSignal = false;
};

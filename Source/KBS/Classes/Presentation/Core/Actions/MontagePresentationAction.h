#pragma once
#include "CoreMinimal.h"
#include "Presentation/Core/PresentationSequenceAction.h"
#include "MontagePresentationAction.generated.h"

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

	// If set, fires FinishExecution when this notify name is hit; montage keeps playing until natural end
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	FName EarlyExitNotifyName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Effects")
	TObjectPtr<USoundBase> SFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Effects")
	TArray<TObjectPtr<UNiagaraSystem>> VFX;
};

UCLASS(BlueprintType, Blueprintable)
class KBS_API UMontagePresentationAction : public UPresentationSequenceAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	FMontageActionContext Context;

protected:
	virtual void OnExecute(EPlaybackMode PlaybackMode) override;
	virtual void OnCleanup() override;
	virtual void StartTimeout(float Duration) override;
	virtual void CancelTimeout() override;

private:
	FTimerHandle TimeoutHandle;
	TObjectPtr<UAudioComponent> ActiveSFX;
	TArray<TObjectPtr<UNiagaraComponent>> ActiveVFX;
	bool bExitedViaSignal = false;

	UAnimInstance* GetAnimInstance() const;

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnMontageEndedCleanup(UAnimMontage* Montage, bool bInterrupted);
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	void StartSFX();
	void StopSFX();
	void StartVFX();
	void StopVFX();
	void StopMontage();
};

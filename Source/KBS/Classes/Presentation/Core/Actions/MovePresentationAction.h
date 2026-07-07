#pragma once
#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Presentation/Core/PresentationSequenceAction.h"
#include "MovePresentationAction.generated.h"

class USoundBase;
class UAudioComponent;
class UNiagaraSystem;
class UNiagaraComponent;

USTRUCT(BlueprintType)
struct FPresentationMoveSegment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Start = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector End = FVector::ZeroVector;

	// Pre-calculated by builder: FVector::Dist(Start, End) / Speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator TargetRotation = FRotator::ZeroRotator;

	FPresentationMoveSegment() = default;
	FPresentationMoveSegment(FVector InStart, FVector InEnd, float InDuration, FRotator InRotation)
		: Start(InStart), End(InEnd), Duration(InDuration), TargetRotation(InRotation) {}
};

USTRUCT(BlueprintType)
struct FMoveActionContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	TObjectPtr<AActor> Actor;

	// Pre-built segments; builder is responsible for filling Duration on each
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	TArray<FPresentationMoveSegment> Path;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Effects")
	TObjectPtr<USoundBase> SFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Effects")
	TArray<TObjectPtr<UNiagaraSystem>> VFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	EPresentationTransitionPolicy TransitionPolicy = EPresentationTransitionPolicy::WaitForCleanup;
};

UCLASS(BlueprintType, Blueprintable)
class KBS_API UMovePresentationAction : public UPresentationSequenceAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	FMoveActionContext Context;

	virtual EPresentationTransitionPolicy GetTransitionPolicy() const override { return Context.TransitionPolicy; }

protected:
	virtual void OnExecute(EPlaybackMode PlaybackMode) override;
	virtual void OnCleanup() override;

private:
	FTSTicker::FDelegateHandle TickerHandle;
	bool OnTick(float DeltaTime);
	void StopTicker();

	int32 CurrentSegmentIndex = 0;
	float SegmentProgress = 0.f;

	TObjectPtr<UAudioComponent> ActiveSFX;
	TArray<TObjectPtr<UNiagaraComponent>> ActiveVFX;

	void ApplyFinalTransform();
	void StartSFX();
	void StopSFX();
	void StartVFX();
	void StopVFX();
};

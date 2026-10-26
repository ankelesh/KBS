#pragma once
#include "CoreMinimal.h"
#include "Presentation/Core/PresentationSequenceAction.h"
#include "VfxPresentationAction.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FVfxActionContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> System;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Duration = 0.f;   // informational; Niagara auto-destroys on completion

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float ZOffset = 0.f;    // usually the owner's collision half-height
};

// Fire-and-forget: spawns a self-managed Niagara system and finishes immediately.
UCLASS(BlueprintType, Blueprintable)
class KBS_API UVfxPresentationAction : public UPresentationSequenceAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVfxActionContext Context;

protected:
	virtual void OnExecute(EPlaybackMode PlaybackMode) override;
	virtual void OnCleanup() override;
};

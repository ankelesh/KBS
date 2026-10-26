#pragma once
#include "CoreMinimal.h"
#include "Presentation/Core/PresentationSequenceAction.h"
#include "FloatingTextPresentationAction.generated.h"

USTRUCT(BlueprintType)
struct FFloatingTextActionContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FloatingText")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FloatingText")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FloatingText")
	FLinearColor Color = FLinearColor::White;
};

// Fire-and-forget: spawns a self-managed AFloatingTextActor and finishes immediately.
UCLASS(BlueprintType, Blueprintable)
class KBS_API UFloatingTextPresentationAction : public UPresentationSequenceAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FloatingText")
	FFloatingTextActionContext Context;

protected:
	virtual void OnExecute(EPlaybackMode PlaybackMode) override;
	virtual void OnCleanup() override;
};

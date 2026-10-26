#pragma once
#include "CoreMinimal.h"
#include "Presentation/Core/PresentationSequenceAction.h"
#include "DespawnPresentationAction.generated.h"

class AUnit;

USTRUCT(BlueprintType)
struct FDespawnActionContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Despawn")
	TObjectPtr<AUnit> Unit;
};

UCLASS(BlueprintType, Blueprintable)
class KBS_API UDespawnPresentationAction : public UPresentationSequenceAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Despawn")
	FDespawnActionContext Context;

protected:
	virtual void OnExecute(EPlaybackMode PlaybackMode) override;
	virtual void OnCleanup() override;
};

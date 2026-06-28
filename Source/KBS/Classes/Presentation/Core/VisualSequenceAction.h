#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Presentation/Core/PresentationSequencePlayer.h"
#include "VisualSequenceAction.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class KBS_API UVisualSequenceAction : public UObject
{
	GENERATED_BODY()

public:
	virtual void Execute(EPlaybackMode PlaybackMode) {}
};

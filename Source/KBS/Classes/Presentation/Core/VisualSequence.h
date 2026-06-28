#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VisualSequence.generated.h"

class UVisualSequenceAction;

UCLASS(BlueprintType)
class KBS_API UVisualSequence : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<TObjectPtr<UVisualSequenceAction>> Actions;
};

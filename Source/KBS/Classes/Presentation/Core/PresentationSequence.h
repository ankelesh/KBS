#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PresentationSequence.generated.h"

class UPresentationSequenceAction;

UCLASS(BlueprintType)
class KBS_API UPresentationSequence : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<TObjectPtr<UPresentationSequenceAction>> Actions;
};

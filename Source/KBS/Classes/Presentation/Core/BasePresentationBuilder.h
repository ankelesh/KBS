#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BasePresentationBuilder.generated.h"

class UPresentationSequence;

UCLASS(Abstract, BlueprintType, Blueprintable)
class KBS_API UBasePresentationBuilder : public UObject
{
	GENERATED_BODY()

public:
	// FromEventId/ToEventId describe the log spine slice to convert (exclusive/inclusive).
	// Invalid FromEventId means "from the start of the spine".
	UFUNCTION(BlueprintNativeEvent, Category = "Presentation")
	UPresentationSequence* Build(FGuid FromEventId, FGuid ToEventId);
	virtual UPresentationSequence* Build_Implementation(FGuid FromEventId, FGuid ToEventId) { return nullptr; }
};

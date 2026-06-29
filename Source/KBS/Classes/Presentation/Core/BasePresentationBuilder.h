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
	UFUNCTION(BlueprintNativeEvent, Category = "Presentation")
	UPresentationSequence* Build();
	virtual UPresentationSequence* Build_Implementation() { return nullptr; }
};

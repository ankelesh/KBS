#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BasePresentationBuilder.generated.h"

class UVisualSequence;

UCLASS(Abstract, BlueprintType, Blueprintable)
class KBS_API UBasePresentationBuilder : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Presentation")
	UVisualSequence* Build();
	virtual UVisualSequence* Build_Implementation() { return nullptr; }
};

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "UnitDragDropOperation.generated.h"

class UUnitDefinition;

UCLASS()
class KBS_API UUnitDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
public:
	UPROPERTY() TObjectPtr<UUnitDefinition> Definition;
};

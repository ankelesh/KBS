#pragma once
#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "Components/ActorComponent.h"
#include "GameMechanics/Units/Components/Config/UnitComponentConfig.h"
#include "UnitComponentEntry.generated.h"

USTRUCT(BlueprintType)
struct KBS_API FUnitComponentEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TSubclassOf<UActorComponent> ComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (BaseStruct = "/Script/KBS.UnitComponentConfig", ExcludeBaseStruct))
	FInstancedStruct Config;
};

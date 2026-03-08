#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "CombatDescriptorDisplayData.generated.h"
USTRUCT(BlueprintType)
struct KBS_API FCombatDescriptorDisplayData
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite)
    FString DescriptorName;
    UPROPERTY(BlueprintReadWrite)
    FString DamageTypes;
    UPROPERTY(BlueprintReadWrite)
    FString TargetType;
    UPROPERTY(BlueprintReadWrite)
    int32 Damage = 0;
};

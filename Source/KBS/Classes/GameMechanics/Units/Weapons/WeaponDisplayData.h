#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "WeaponDisplayData.generated.h"
USTRUCT(BlueprintType)
struct KBS_API FWeaponDisplayData
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite)
    FString WeaponName;
    UPROPERTY(BlueprintReadWrite)
    FString DamageTypes;  
    UPROPERTY(BlueprintReadWrite)
    FString TargetType;  
    UPROPERTY(BlueprintReadWrite)
    int32 Damage;
};

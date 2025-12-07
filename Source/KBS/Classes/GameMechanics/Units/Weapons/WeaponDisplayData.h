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
    FString DamageTypes; // "Physical + Fire + Holy", etc.

    UPROPERTY(BlueprintReadWrite)
    FString TargetType; // "Adjacent", "Ranged", "Line", etc.

    UPROPERTY(BlueprintReadWrite)
    int32 Damage;
};
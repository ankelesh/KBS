#pragma once
#include "CoreMinimal.h"


UENUM(BlueprintType)
enum class EEffectType : uint8
{
	Positive UMETA(DisplayName = "Positive"),
	Negative UMETA(DisplayName = "Negative"),
	Immutable UMETA(DisplayName = "Immutable"),
	Neutral UMETA(DisplayName = "Neutral")
};





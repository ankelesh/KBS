#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EDamageSource : uint8
{
	Physical UMETA(DisplayName = "Physical"),
	Fire UMETA(DisplayName = "Fire"),
	Earth UMETA(DisplayName = "Earth"),
	Air UMETA(DisplayName = "Air"),
	Water UMETA(DisplayName = "Water"),
	Life UMETA(DisplayName = "Life"),
	Death UMETA(DisplayName = "Death"),
	Mind UMETA(DisplayName = "Mind")
};

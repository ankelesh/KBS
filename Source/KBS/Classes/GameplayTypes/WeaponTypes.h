#pragma once
#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponDesignation : uint8
{
	AllPurpose  UMETA(DisplayName = "All Purpose"),
	AutoAttacks UMETA(DisplayName = "Auto Attacks Only"),
	Spells      UMETA(DisplayName = "Spells Only"),
};

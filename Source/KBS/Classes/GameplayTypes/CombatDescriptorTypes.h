#pragma once
#include "CoreMinimal.h"
#include "CombatDescriptorTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatDescriptorDesignation : uint8
{
	AllPurpose  UMETA(DisplayName = "All Purpose"),
	AutoAttacks UMETA(DisplayName = "Auto Attacks Only"),
	Spells      UMETA(DisplayName = "Spells Only"),
};

UENUM(BlueprintType)
enum class EMagnitudePolicy : uint8
{
	// Runs damage calculation and application phase
	Damage UMETA(DisplayName = "Damage"),
	// Runs heal calculation and application phase
	Heal   UMETA(DisplayName = "Heal"),
	// Skips damage calculation and application phase entirely
	None   UMETA(DisplayName = "None"),
};

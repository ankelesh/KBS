#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAbilitySlotState : uint8
{
	Selected,   // Available and currently selected (highlighted frame)
	Enabled,    // Available but not selected (default state)
	Disabled,   // Visible but unavailable (greyed out, button disabled)
	Hidden      // Collapsed, not visible
};

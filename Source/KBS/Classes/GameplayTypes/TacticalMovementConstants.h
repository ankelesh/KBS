#pragma once
#include "CoreMinimal.h"

struct KBS_API FTacMovementConstants
{
	static constexpr float ModelForwardOffset = -90.0f;
	static constexpr float AttackerDefaultYaw = 0.0f;
	static constexpr float DefenderDefaultYaw = 180.0f;
	static constexpr float DefaultRotationDuration = 0.15f;
	static constexpr float FallbackMovementDuration = 0.3f;
};

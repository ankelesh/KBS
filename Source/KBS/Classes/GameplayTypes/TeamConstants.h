#pragma once
#include "CoreMinimal.h"


UENUM(BlueprintType)
enum class ETeamSide : uint8
{
	Attacker UMETA(DisplayName = "Attacker"),
	Defender UMETA(DisplayName = "Defender")
};
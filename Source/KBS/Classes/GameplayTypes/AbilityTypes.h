#pragma once
#include "CoreMinimal.h"
#include "AbilityTypes.generated.h"
class UBattleEffect;
class AUnit;
UENUM(BlueprintType)
enum class EDefaultAbilitySlot : uint8
{
	Attack UMETA(DisplayName = "Attack"),
	Move UMETA(DisplayName = "Move"),
	Wait UMETA(DisplayName = "Wait"),
	Defend UMETA(DisplayName = "Defend"),
	Flee UMETA(DisplayName = "Flee")
};
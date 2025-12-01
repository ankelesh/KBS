#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EDamageSource : uint8
{
	None UMETA(DisplayName = "None"),
	Physical UMETA(DisplayName = "Physical"),
	Fire UMETA(DisplayName = "Fire"),
	Earth UMETA(DisplayName = "Earth"),
	Air UMETA(DisplayName = "Air"),
	Water UMETA(DisplayName = "Water"),
	Life UMETA(DisplayName = "Life"),
	Death UMETA(DisplayName = "Death"),
	Mind UMETA(DisplayName = "Mind")
};

UENUM(BlueprintType)
enum class ETargetReach : uint8
{
	None UMETA(DisplayName = "None"),
	ClosestEnemies UMETA(DisplayName = "Closest Enemies"),
	AnyEnemy UMETA(DisplayName = "Any Enemy"),
	AllEnemies UMETA(DisplayName = "All Enemies"),
	Area UMETA(DisplayName = "Area"),
	AnyFriendly UMETA(DisplayName = "Any Friendly"),
	AllFriendlies UMETA(DisplayName = "All Friendlies"),
	EmptyCell UMETA(DisplayName = "Empty Cell"),
	EmptyCellOrFriendly UMETA(DisplayName = "Empty Cell Or Friendly")
};

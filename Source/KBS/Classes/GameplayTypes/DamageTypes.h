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
	Self UMETA(DisplayName = "Self"),
	ClosestEnemies UMETA(DisplayName = "Closest Enemies"),
	AnyEnemy UMETA(DisplayName = "Any Enemy"),
	AllEnemies UMETA(DisplayName = "All Enemies"),
	Area UMETA(DisplayName = "Area"),
	AnyFriendly UMETA(DisplayName = "Any Friendly"),
	AllFriendlies UMETA(DisplayName = "All Friendlies"),
	EmptyCell UMETA(DisplayName = "Empty Cell"),
	EmptyCellOrFriendly UMETA(DisplayName = "Empty Cell Or Friendly"),
	Movement UMETA(DisplayName = "Movement"),
	AnyCorpse UMETA(DisplayName = "Any Corpse"),
	FriendlyCorpse UMETA(DisplayName = "Friendly Corpse"),
	EnemyCorpse UMETA(DisplayName = "Enemy Corpse"),
	AnyNonBlockedCorpse UMETA(DisplayName = "Any Non-Blocked Corpse"),
	FriendlyNonBlockedCorpse UMETA(DisplayName = "Friendly Non-Blocked Corpse"),
	EnemyNonBlockedCorpse UMETA(DisplayName = "Enemy Non-Blocked Corpse")
};
UENUM(BlueprintType)
enum class EShapeLayering : uint8
{
	GroundArea UMETA(DisplayName = "Ground Area"),
	AirArea UMETA(DisplayName = "Air Area"),
	BothLayerArea UMETA(DisplayName = "Both Layers Area"),
};

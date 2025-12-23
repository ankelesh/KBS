#include "GameplayTypes/AbilityTypesLibrary.h"
FString UAbilityTypesLibrary::TargetReachToString(ETargetReach Reach)
{
	static const TMap<ETargetReach, FString> TargetReachStrings = {
		{ ETargetReach::None, TEXT("None") },
		{ ETargetReach::Self, TEXT("Self") },
		{ ETargetReach::ClosestEnemies, TEXT("Closest Enemies") },
		{ ETargetReach::AnyEnemy, TEXT("Any Enemy") },
		{ ETargetReach::AllEnemies, TEXT("All Enemies") },
		{ ETargetReach::Area, TEXT("Area") },
		{ ETargetReach::AnyFriendly, TEXT("Any Friendly") },
		{ ETargetReach::AllFriendlies, TEXT("All Friendlies") },
		{ ETargetReach::EmptyCell, TEXT("Empty Cell") },
		{ ETargetReach::EmptyCellOrFriendly, TEXT("Empty Cell Or Friendly") }
	};
	const FString* Found = TargetReachStrings.Find(Reach);
	return Found ? *Found : TEXT("Unknown");
}

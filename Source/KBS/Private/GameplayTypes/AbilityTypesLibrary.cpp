#include "GameplayTypes/AbilityTypesLibrary.h"
#include "GameMechanics/Tactical/Grid/Components/GridHighlightComponent.h"

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

EHighlightType UAbilityTypesLibrary::TargetReachToHighlightType(ETargetReach Reach)
{
	switch (Reach)
	{
	case ETargetReach::Movement:
		return EHighlightType::Movement;

	case ETargetReach::AnyFriendly:
	case ETargetReach::AllFriendlies:
	case ETargetReach::EmptyCellOrFriendly:
	case ETargetReach::FriendlyCorpse:
	case ETargetReach::FriendlyNonBlockedCorpse:
		return EHighlightType::Friendly;

	case ETargetReach::EmptyCell:
		return EHighlightType::Movement;

	case ETargetReach::Self:
		return EHighlightType::CurrentSelected;

	// All enemy-targeting abilities
	case ETargetReach::ClosestEnemies:
	case ETargetReach::AnyEnemy:
	case ETargetReach::AllEnemies:
	case ETargetReach::Area:
	case ETargetReach::AnyCorpse:
	case ETargetReach::EnemyCorpse:
	case ETargetReach::AnyNonBlockedCorpse:
	case ETargetReach::EnemyNonBlockedCorpse:
	default:
		return EHighlightType::Attack;
	}
}

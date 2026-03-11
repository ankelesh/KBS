#include "GameplayTypes/TargetingDescriptor.h"

ETargetFastCheck FTargetingDescriptor::GetFastCheckHint() const
{
	switch (Strategy)
	{
	case ETargetingStrategy::Single:
	case ETargetingStrategy::All:
	case ETargetingStrategy::Area:     return ETargetFastCheck::Affiliation;
	case ETargetingStrategy::EmptyCell: return ETargetFastCheck::EmptyCell;
	case ETargetingStrategy::Corpse:    return ETargetFastCheck::Corpse;
	case ETargetingStrategy::Movement:  return ETargetFastCheck::Movement;
	default:                            return ETargetFastCheck::Full;
	}
}

bool FTargetingDescriptor::operator==(const FTargetingDescriptor& other) const
{
	return GetTypeHash(*this) == GetTypeHash(other);
}

FTargetingDescriptor FTargetingDescriptor::FromReach(ETargetReach Reach)
{
	using S = ETargetingStrategy;
	using A = ETargetAffiliation;
	using M = EMovementPattern;
	using L = EMovementLayer;

	static const TMap<ETargetReach, FTargetingDescriptor> Table = {
		{ ETargetReach::None,                    { S::None                                                    } },
		{ ETargetReach::Self,                    { S::Self                                                    } },
		{ ETargetReach::ClosestEnemies,          { S::Closest,   A::Enemy                                     } },
		{ ETargetReach::AnyEnemy,                { S::Single,    A::Enemy                                     } },
		{ ETargetReach::AllEnemies,              { S::All,       A::Enemy                                     } },
		{ ETargetReach::AnyFriendly,             { S::Single,    A::Friendly                                  } },
		{ ETargetReach::AllFriendlies,           { S::All,       A::Friendly                                  } },
		{ ETargetReach::Area,                    { S::Area,      A::Any                                       } },
		{ ETargetReach::AreaFriendly,            { S::Area,      A::Friendly                                  } },
		{ ETargetReach::AreaEnemy,               { S::Area,      A::Enemy                                     } },
		{ ETargetReach::EmptyCell,               { S::EmptyCell                                               } },
		{ ETargetReach::EmptyCellOrFriendly,     { S::Single,    A::Friendly,  M::Orthogonal, L::Ground, true, true } },
		{ ETargetReach::AnyCorpse,               { S::Corpse,    A::Any,       M::Orthogonal, L::Ground, true  } },
		{ ETargetReach::FriendlyCorpse,          { S::Corpse,    A::Friendly,  M::Orthogonal, L::Ground, true  } },
		{ ETargetReach::EnemyCorpse,             { S::Corpse,    A::Enemy,     M::Orthogonal, L::Ground, true  } },
		{ ETargetReach::AnyNonBlockedCorpse,     { S::Corpse,    A::Any,       M::Orthogonal, L::Ground, false } },
		{ ETargetReach::FriendlyNonBlockedCorpse,{ S::Corpse,    A::Friendly,  M::Orthogonal, L::Ground, false } },
		{ ETargetReach::EnemyNonBlockedCorpse,   { S::Corpse,    A::Enemy,     M::Orthogonal, L::Ground, false } },
		{ ETargetReach::AnyUnit,                 { S::Single,    A::Any                                       } },
		{ ETargetReach::AllUnits,                { S::All,       A::Any                                       } },
		{ ETargetReach::ClosestFriendly,         { S::Closest,   A::Friendly                                  } },
		{ ETargetReach::EmptyCellOrEnemy,        { S::Single,    A::Enemy,     M::Orthogonal, L::Ground, true, true } },
		{ ETargetReach::GroundMove,              { S::Movement,  A::Any,       M::Orthogonal, L::Ground  } },
		{ ETargetReach::TeleportMovement,        { S::Movement,  A::Any,       M::AnyToAny,   L::Ground  } },
		{ ETargetReach::Fly,                     { S::Movement,  A::Any,       M::AnyToAny,   L::Air     } },
		{ ETargetReach::FlyChangeLayer,          { S::Movement,  A::Any,       M::AnyToAny,   L::CrossLayer } },
		{ ETargetReach::RestrictedFly,           { S::Movement,  A::Any,       M::Orthogonal, L::Air     } },
		{ ETargetReach::RestrictedFlyChangeLayer,{ S::Movement,  A::Any,       M::Orthogonal, L::CrossLayer } },
	};

	const FTargetingDescriptor* Found = Table.Find(Reach);
	checkf(Found, TEXT("FTargetingDescriptor::FromReach - unregistered ETargetReach value %d"), (int32)Reach);
	return *Found;
}

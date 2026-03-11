#pragma once
#include "GameplayTypes/TargetingDescriptor.h"

class AUnit;
struct FTacCoordinates;

namespace QueryPredicates
{
	typedef TFunction<bool(const AUnit* /* Source unit */, const AUnit* /* Occupant unit */,
						  const FTacCoordinates& /* Passed coord */)> FCellFilterPredicate;
	typedef TFunction<bool(const FTacCoordinates& /* Passed coord */, bool /* Is blocked */, const AUnit* /* Top corpse */,
						  int32 /* Num corpses */)> FCorpseFilterPredicate;
}


namespace TargetingPredicates
{
	QueryPredicates::FCellFilterPredicate MovePredicateFactory(bool bIsMulticell);

	bool EmptyCellPredicate(const AUnit* SourceUnit, const AUnit* Occupant, const FTacCoordinates& Cell);
	bool EmptyNotFlankPredicate(const AUnit* SourceUnit, const AUnit* Occupant, const FTacCoordinates& Cell);

	QueryPredicates::FCellFilterPredicate AffiliationPredicateFactory(const FTargetingDescriptor& Desc);

	QueryPredicates::FCorpseFilterPredicate CorpseAffiliationPredicateFactory(
		AUnit* SourceUnit, const FTargetingDescriptor& Desc);

	extern QueryPredicates::FCellFilterPredicate AirMovePredicate;
}

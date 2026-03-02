#pragma once
#include "TargetingTypes.h"

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

	QueryPredicates::FCellFilterPredicate AffiliationPredicateFactory(EAffiliationFilter Filter, bool bAllowEmpty,
	                                                                  bool bAllowFlank, bool bAllowDelayed=true);

	QueryPredicates::FCorpseFilterPredicate CorpseAffiliationPredicateFactory(
		AUnit* SourceUnit, EAffiliationFilter Filter, bool bAllowBlocked);

	extern QueryPredicates::FCellFilterPredicate AirMovePredicate;
}

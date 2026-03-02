#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridQueryPredicates.h"
#include "GameMechanics/Units/Unit.h"

namespace TargetingPredicates
{
	QueryPredicates::FCellFilterPredicate MovePredicateFactory(bool bIsMulticell)
	{
		if (bIsMulticell)
			return [](const AUnit* SourceUnit, const AUnit* Occupant, const FTacCoordinates& Cell)-> bool
			{
				if (Occupant)
				{
					check(!Occupant->IsDead());
					if (Occupant->GetGridMetadata().IsEnemy(SourceUnit->GetGridMetadata()))
						return false;
					if (Occupant->GetGridMetadata().IsMultiCell())
						return false;
				}
				return true;
			};
		else
			return [](const AUnit* SourceUnit, const AUnit* Occupant, const FTacCoordinates& Cell)-> bool
			{
				return !Occupant;
			};
	}

	bool EmptyCellPredicate(const AUnit* SourceUnit, const AUnit* Occupant, const FTacCoordinates& Cell)
	{
		return !Occupant;
	}

	bool EmptyNotFlankPredicate(const AUnit* SourceUnit, const AUnit* Occupant, const FTacCoordinates& Cell)
	{
		return !Occupant && !Cell.IsFlankCell();
	}

	QueryPredicates::FCellFilterPredicate AffiliationPredicateFactory(EAffiliationFilter Filter, bool bAllowEmpty,
	                                                                  bool bAllowFlank, bool bAllowDelayed)
	{
		return [=](const AUnit* SourceUnit, const AUnit* Occupant, const FTacCoordinates& Cell)
		{
			if (Cell.IsFlankCell() && !bAllowFlank)
				return false;
			if (Occupant)
			{
				if (!bAllowDelayed && Occupant->GetStats().Status.IsFlankDelayed())
					return false;
				switch (Filter)
				{
				case EAffiliationFilter::Enemy:
					return Occupant->GetGridMetadata().IsEnemy(SourceUnit->GetGridMetadata());
				case EAffiliationFilter::Friendly:
					return !Occupant->GetGridMetadata().IsEnemy(SourceUnit->GetGridMetadata());
				case EAffiliationFilter::Any:
				default:
					return true;
				}
			}

			return bAllowEmpty;
		};
	}

	QueryPredicates::FCorpseFilterPredicate CorpseAffiliationPredicateFactory(
		AUnit* SourceUnit, EAffiliationFilter Filter, bool bAllowBlocked)
	{
		return [=](const FTacCoordinates& Coords, bool bIsBlocked, const AUnit* TopCorpse,
		           int32 CorpseNum) -> bool
		{
			if (bIsBlocked && !bAllowBlocked)
				return false;
			if (TopCorpse)
			{
				switch (Filter)
				{
				case EAffiliationFilter::Enemy:
					return SourceUnit->GetGridMetadata().IsEnemy(TopCorpse->GetGridMetadata());
				case EAffiliationFilter::Friendly:
					return !SourceUnit->GetGridMetadata().IsEnemy(TopCorpse->GetGridMetadata());
				case EAffiliationFilter::Any:
				default:
					return true;
				}
			}
			return true;
		};
	}

	QueryPredicates::FCellFilterPredicate AirMovePredicate = [](const AUnit* SourceUnit, const AUnit* Occupant,
	                                                            FTacCoordinates Coords)
	{
		if (Occupant)
		{
			if (Occupant->GetGridMetadata().IsEnemy(SourceUnit->GetGridMetadata()))
				return false;
		}
		return true;
	};
}

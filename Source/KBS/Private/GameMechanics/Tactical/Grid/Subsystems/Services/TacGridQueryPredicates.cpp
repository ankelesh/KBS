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

	QueryPredicates::FCellFilterPredicate AffiliationPredicateFactory(const FTargetingDescriptor& Desc)
	{
		const ETargetAffiliation Filter = Desc.Affiliation;
		const bool bAllowEmpty = Desc.bAllowEmpty;
		const bool bAllowFlank = Desc.bAllowFlank;
		const bool bAllowDelayed = (Desc.Strategy != ETargetingStrategy::Closest);
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
				case ETargetAffiliation::Enemy:
					return Occupant->GetGridMetadata().IsEnemy(SourceUnit->GetGridMetadata());
				case ETargetAffiliation::Friendly:
					return !Occupant->GetGridMetadata().IsEnemy(SourceUnit->GetGridMetadata());
				case ETargetAffiliation::Any:
				default:
					return true;
				}
			}

			return bAllowEmpty;
		};
	}

	QueryPredicates::FCorpseFilterPredicate CorpseAffiliationPredicateFactory(
		AUnit* SourceUnit, const FTargetingDescriptor& Desc)
	{
		const ETargetAffiliation Filter = Desc.Affiliation;
		const bool bAllowBlocked = Desc.bAllowCoveredCorpse;
		return [=](const FTacCoordinates& Coords, bool bIsBlocked, const AUnit* TopCorpse,
		           int32 CorpseNum) -> bool
		{
			if (bIsBlocked && !bAllowBlocked)
				return false;
			if (TopCorpse)
			{
				switch (Filter)
				{
				case ETargetAffiliation::Enemy:
					return SourceUnit->GetGridMetadata().IsEnemy(TopCorpse->GetGridMetadata());
				case ETargetAffiliation::Friendly:
					return !SourceUnit->GetGridMetadata().IsEnemy(TopCorpse->GetGridMetadata());
				case ETargetAffiliation::Any:
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

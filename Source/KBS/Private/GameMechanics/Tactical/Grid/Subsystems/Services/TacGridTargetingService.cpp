// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/Grid/Algo/TacGridAlgo.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/TargetingDescriptor.h"
#include "GameplayTypes/FlankCellDefinitions.h"
#include "GameplayTypes/CombatTypes.h"

using namespace TargetingPredicates;

UTacGridTargetingService::UTacGridTargetingService()
{
}

void UTacGridTargetingService::Initialize(UGridDataManager* InDataManager)
{
	DataManager = InDataManager;
	checkf(DataManager, TEXT("TacGridTargetingService: DataManager must be valid after Initialize"));
}

// === Public interface ===

TArray<FTacCoordinates> UTacGridTargetingService::GetValidTargetCells(AUnit* Unit, FTargetingDescriptor Desc) const
{
	check(Unit);
	TArray<FTacCoordinates> TargetCells;
	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();

	switch (Desc.Strategy)
	{
	case ETargetingStrategy::None:
		break;
	case ETargetingStrategy::Self:
		TargetCells.Append(Metadata.GetCells());
		break;
	case ETargetingStrategy::Closest:
		TargetCells = GetClosestEnemyCells(Unit, Desc);
		break;
	case ETargetingStrategy::Single:
	case ETargetingStrategy::All:
	case ETargetingStrategy::Area:
		TargetCells = GetCellsByAffiliation(Unit, Desc);
		break;
	case ETargetingStrategy::EmptyCell:
		DataManager->FilterCells(Unit, Metadata.Coords.Layer,
		                         Desc.bAllowFlank ? EmptyCellPredicate : EmptyNotFlankPredicate, TargetCells);
		break;
	case ETargetingStrategy::Movement:
		TargetCells = GetValidMovementCells(Unit, Desc);
		break;
	case ETargetingStrategy::Corpse:
		TargetCells = GetCorpseCellsByAffiliation(Unit, Desc);
		break;
	default:
		UE_LOG(LogTacGrid, Warning, TEXT("Unknown targeting strategy: %d"), (int32)Desc.Strategy);
	}
	UE_LOG(LogTacGrid, Log, TEXT("GetValidTargetCells: %s Strategy=%d -> %d cells"), *Unit->GetLogName(),
	       (int32)Desc.Strategy, TargetCells.Num());
	return TargetCells;
}

TArray<FTacCoordinates> UTacGridTargetingService::GetValidTargetCells(AUnit* Unit, ETargetReach Reach) const
{
	return GetValidTargetCells(Unit, FTargetingDescriptor::FromReach(Reach));
}

TArray<AUnit*> UTacGridTargetingService::GetValidTargetUnits(AUnit* Unit, FTargetingDescriptor Desc,
                                                             bool bIncludeSelf) const
{
	check(Unit);
	auto TargetCells = GetValidTargetCells(Unit, Desc);
	if (!bIncludeSelf)
	{
		TargetCells.Remove(Unit->GetGridMetadata().Coords);
		if (Unit->GetGridMetadata().HasExtraCell())
			TargetCells.Remove(Unit->GetGridMetadata().ExtraCell);
	}
	auto TargetUnits{KbsAlgo::ExtractUnits<TArray>(TargetCells, DataManager)};
	UE_LOG(LogTacGrid, Log, TEXT("GetValidTargetUnits: %s Strategy=%d -> %d units"), *Unit->GetLogName(),
	       (int32)Desc.Strategy, TargetUnits.Num());
	return TargetUnits;
}

TArray<AUnit*> UTacGridTargetingService::GetValidTargetUnits(AUnit* Unit, ETargetReach Reach,
                                                             bool bIncludeSelf) const
{
	return GetValidTargetUnits(Unit, FTargetingDescriptor::FromReach(Reach), bIncludeSelf);
}

FResolvedTargets UTacGridTargetingService::ResolveTargetsFromClick(AUnit* SourceUnit, FTacCoordinates ClickedCell,
                                                                   FTargetingDescriptor Desc,
                                                                   const FAreaShape* AreaShape) const
{
	check(SourceUnit);

	TArray<FTacCoordinates> Cells;
	switch (Desc.Strategy)
	{
	case ETargetingStrategy::All:
		Cells = GetValidTargetCells(SourceUnit, Desc);
		return BuildTargetsFromCells(ClickedCell, Cells);
	case ETargetingStrategy::Area:
		if (AreaShape)
			Cells = GetCellsInArea(SourceUnit, ClickedCell, Desc, *AreaShape);
		return BuildTargetsFromCells(ClickedCell, Cells);
	case ETargetingStrategy::Corpse:
		return BuildTargetFromCell(ClickedCell,
		                           TestCell(ClickedCell, CorpseAffiliationPredicateFactory(SourceUnit, Desc)));
	case ETargetingStrategy::Closest:
		return BuildTargetFromCell(ClickedCell, CanTargetClosestCell(SourceUnit, Desc, ClickedCell));
	case ETargetingStrategy::Single:
		return BuildTargetFromCell(ClickedCell,
		                           TestCell(SourceUnit, ClickedCell, AffiliationPredicateFactory(Desc)));
	case ETargetingStrategy::EmptyCell:
		return BuildTargetFromCell(ClickedCell, TestCell(SourceUnit, ClickedCell,
		                           Desc.bAllowFlank ? EmptyCellPredicate : EmptyNotFlankPredicate));
	case ETargetingStrategy::Self:
		return BuildTargetFromCell(ClickedCell, CanSelfTarget(SourceUnit, ClickedCell));
	case ETargetingStrategy::Movement:
		return BuildTargetFromCell(ClickedCell, TestMovementCell(SourceUnit, ClickedCell, Desc));
	default:
		UE_LOG(LogTacGrid, Warning, TEXT("Unknown targeting strategy: %d"), (int32)Desc.Strategy);
	}

	return FResolvedTargets::MakeEmpty();
}

FResolvedTargets UTacGridTargetingService::ResolveTargetsFromClick(AUnit* SourceUnit, FTacCoordinates ClickedCell,
                                                                   ETargetReach Reach,
                                                                   const FAreaShape* AreaShape) const
{
	return ResolveTargetsFromClick(SourceUnit, ClickedCell, FTargetingDescriptor::FromReach(Reach), AreaShape);
}

bool UTacGridTargetingService::HasValidTargetAtCell(AUnit* Source, FTacCoordinates TargetCell,
                                                    FTargetingDescriptor Desc) const
{
	if (!CheckUnitAndData(Source)) return false;
	if (!TargetCell.IsValidCell()) return false;

	switch (Desc.Strategy)
	{
	case ETargetingStrategy::None:
		return false;
	case ETargetingStrategy::Self:
		return CanSelfTarget(Source, TargetCell);
	case ETargetingStrategy::Closest:
		return CanTargetClosestCell(Source, Desc, TargetCell);
	case ETargetingStrategy::Single:
	case ETargetingStrategy::All:
	case ETargetingStrategy::Area:
		return TestCell(Source, TargetCell, AffiliationPredicateFactory(Desc));
	case ETargetingStrategy::EmptyCell:
		return TestCell(Source, TargetCell, Desc.bAllowFlank ? EmptyCellPredicate : EmptyNotFlankPredicate);
	case ETargetingStrategy::Movement:
		return TestMovementCell(Source, TargetCell, Desc);
	case ETargetingStrategy::Corpse:
		return TestCell(TargetCell, CorpseAffiliationPredicateFactory(Source, Desc));
	default:
		UE_LOG(LogTacGrid, Warning, TEXT("Unknown targeting strategy: %d in HasValidTargetAtCell"),
		       (int32)Desc.Strategy);
	}
	return false;
}

bool UTacGridTargetingService::HasValidTargetAtCell(AUnit* Source, FTacCoordinates TargetCell,
                                                    ETargetReach Reach) const
{
	return HasValidTargetAtCell(Source, TargetCell, FTargetingDescriptor::FromReach(Reach));
}

bool UTacGridTargetingService::HasAnyValidTargets(AUnit* Source, FTargetingDescriptor Desc) const
{
	check(Source);
	if (Desc.Strategy == ETargetingStrategy::None)
		return false;
	if (Desc.Strategy == ETargetingStrategy::Self)
		return true;

	const FUnitGridMetadata& Metadata = Source->GetGridMetadata();
	const ETeamSide TeamSide = Metadata.Team;

	switch (Desc.GetFastCheckHint())
	{
	case ETargetFastCheck::Affiliation:
		{
			UBattleTeam* EnemyTeam    = DataManager->GetTeamBySide(
				TeamSide == ETeamSide::Attacker ? ETeamSide::Defender : ETeamSide::Attacker);
			UBattleTeam* FriendlyTeam = DataManager->GetTeamBySide(TeamSide);
			auto EnemyHas    = [&]{ return EnemyTeam->IsAnyUnitAlive()           && EnemyTeam->IsAnyUnitOnField();            };
			auto FriendlyHas = [&]{ return FriendlyTeam->IsOtherUnitAlive(Source) && FriendlyTeam->IsOtherUnitOnField(Source); };
			switch (Desc.Affiliation)
			{
			case ETargetAffiliation::Enemy:    return EnemyHas();
			case ETargetAffiliation::Friendly: return FriendlyHas();
			case ETargetAffiliation::Any:      return EnemyHas() || FriendlyHas();
			}
		}
		break;
	case ETargetFastCheck::EmptyCell:
		return DataManager->TestCells(Source, Metadata.Coords.Layer,
		                              Desc.bAllowFlank ? EmptyCellPredicate : EmptyNotFlankPredicate);
	case ETargetFastCheck::Corpse:
		return DataManager->TestCorpseCells(Source, CorpseAffiliationPredicateFactory(Source, Desc));
	case ETargetFastCheck::Movement:
		{
			ETacGridLayer QueryLayer;
			if (Desc.MovementLayer == EMovementLayer::CrossLayer)
				QueryLayer = (Metadata.Coords.Layer == ETacGridLayer::Ground) ? ETacGridLayer::Air : ETacGridLayer::Ground;
			else if (Desc.MovementLayer == EMovementLayer::Air)
				QueryLayer = ETacGridLayer::Air;
			else
				QueryLayer = ETacGridLayer::Ground;

			if (QueryLayer == ETacGridLayer::Air && Desc.MovementPattern == EMovementPattern::AnyToAny)
				return DataManager->TestCells(Source, ETacGridLayer::Air, AirMovePredicate);
			if (QueryLayer == ETacGridLayer::Ground && !Metadata.IsMultiCell() &&
				Desc.MovementPattern == EMovementPattern::Orthogonal)
				return DataManager->TestCells(Source, ETacGridLayer::Ground, MovePredicateFactory(false));
		}
		break;
	default:
		break;
	}

	return GetValidTargetCells(Source, Desc).Num() > 0;
}

bool UTacGridTargetingService::HasAnyValidTargets(AUnit* Source, ETargetReach Reach) const
{
	return HasAnyValidTargets(Source, FTargetingDescriptor::FromReach(Reach));
}

// === Private helpers — targeting ===

TArray<FTacCoordinates> UTacGridTargetingService::GetClosestEnemyCells(AUnit* Unit,
                                                                        const FTargetingDescriptor& Desc) const
{
	if (!CheckUnitAndData(Unit)) return {};
	auto EnemyPredicate = AffiliationPredicateFactory(Desc);
	TSet<FTacCoordinates> FoundEnemyCells = KbsAlgo::CollectAdjacentCellsIf<TSet>(
		Unit, DataManager.Get(), EnemyPredicate, true);
	if (FTacCoordinates BlockedCell = FFlankCellDefinitions::GetEntranceBlockedCell(Unit->GetGridMetadata().Coords);
		BlockedCell.IsValidCell())
	{
		FoundEnemyCells.Remove(BlockedCell);
	}
	TArray<FTacCoordinates> Result;
	Algo::Copy(FoundEnemyCells, Result);
	return Result;
}

bool UTacGridTargetingService::CanTargetClosestCell(AUnit* SourceUnit, const FTargetingDescriptor& Desc,
                                                    FTacCoordinates Cell) const
{
	if (!CheckUnitAndData(SourceUnit)) return false;
	if (FTacCoordinates BlockedCell = FFlankCellDefinitions::GetEntranceBlockedCell(
			SourceUnit->GetGridMetadata().Coords);
		BlockedCell.IsValidCell())
		if (Cell == BlockedCell)
			return false;
	auto Predicate = AffiliationPredicateFactory(Desc);
	return KbsAlgo::AdjacentContains(SourceUnit, DataManager, Cell, Predicate);
}

bool UTacGridTargetingService::CanSelfTarget(AUnit* Source, FTacCoordinates Coord) const
{
	check(Source);
	if (!Coord.IsValidCell()) return false;
	if (Source->GetGridMetadata().Coords == Coord)
		return true;
	if (Source->GetGridMetadata().HasExtraCell() && Source->GetGridMetadata().ExtraCell == Coord)
		return true;
	return false;
}

TArray<FTacCoordinates> UTacGridTargetingService::GetCellsByAffiliation(AUnit* SourceUnit,
                                                                         const FTargetingDescriptor& Desc) const
{
	if (!CheckUnitAndData(SourceUnit)) return {};
	TArray<FTacCoordinates> Result;
	auto AffiliationPredicate = AffiliationPredicateFactory(Desc);
	DataManager->FilterCells(SourceUnit, ETacGridLayer::Ground, AffiliationPredicate, Result);
	DataManager->FilterCells(SourceUnit, ETacGridLayer::Air, AffiliationPredicate, Result);
	return Result;
}

TArray<FTacCoordinates> UTacGridTargetingService::GetCorpseCellsByAffiliation(AUnit* Unit,
                                                                               const FTargetingDescriptor& Desc) const
{
	if (!CheckUnitAndData(Unit)) return {};
	TArray<FTacCoordinates> Result;
	DataManager->FilterCorpseCells(Unit, CorpseAffiliationPredicateFactory(Unit, Desc), Result);
	return Result;
}

TArray<FTacCoordinates> UTacGridTargetingService::GetCellsInArea(AUnit* SourceUnit, FTacCoordinates CenterCell,
                                                                  const FTargetingDescriptor& Desc,
                                                                  const FAreaShape& AreaShape) const
{
	if (!CheckUnitAndData(SourceUnit)) return {};
	auto AffiliationPredicate = AffiliationPredicateFactory(Desc);
	return KbsAlgo::CollectCellsInArea<TArray>(SourceUnit, DataManager, AffiliationPredicate, CenterCell, AreaShape);
}

// === Private helpers — movement ===

TArray<FTacCoordinates> UTacGridTargetingService::GetValidMovementCells(AUnit* Unit,
                                                                         const FTargetingDescriptor& Desc) const
{
	if (!CheckUnitAndData(Unit)) return {};
	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	if (Metadata.IsMultiCell() && Metadata.HasExtraCell())
		return GetMultiCellMoveCells(Unit, Desc);
	return GetSingleCellMoveCells(Unit, Desc);
}

TArray<FTacCoordinates> UTacGridTargetingService::GetSingleCellMoveCells(AUnit* Unit,
                                                                          const FTargetingDescriptor& Desc) const
{
	switch (Desc.MovementPattern)
	{
	case EMovementPattern::Orthogonal: return GetSingleCellOrthogonal(Unit, Desc);
	case EMovementPattern::AnyToAny:   return GetSingleCellAnyToAny  (Unit, Desc);
	case EMovementPattern::Linear:     return GetSingleCellLinear    (Unit, Desc);
	}
	return {};
}

TArray<FTacCoordinates> UTacGridTargetingService::GetMultiCellMoveCells(AUnit* Unit,
                                                                         const FTargetingDescriptor& Desc) const
{
	switch (Desc.MovementPattern)
	{
	case EMovementPattern::Orthogonal: return GetMultiCellOrthogonal(Unit, Desc);
	case EMovementPattern::AnyToAny:   return GetMultiCellAnyToAny  (Unit, Desc);
	case EMovementPattern::Linear:     return GetMultiCellLinear    (Unit, Desc);
	}
	return {};
}

TArray<FTacCoordinates> UTacGridTargetingService::GetSingleCellOrthogonal(AUnit* Unit,
                                                                           const FTargetingDescriptor& Desc) const
{
	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	const ETacGridLayer UnitLayer = Metadata.Coords.Layer;
	const ETacGridLayer TargetLayer = (Desc.MovementLayer == EMovementLayer::CrossLayer)
		? (UnitLayer == ETacGridLayer::Ground ? ETacGridLayer::Air : ETacGridLayer::Ground)
		: (Desc.MovementLayer == EMovementLayer::Air ? ETacGridLayer::Air : ETacGridLayer::Ground);
	auto Pred = (TargetLayer == ETacGridLayer::Air) ? AirMovePredicate : MovePredicateFactory(false);

	TArray<FTacCoordinates> Result;
	if (TargetLayer == UnitLayer)
	{
		Result = KbsAlgo::CollectAdjacentCellsIf<TArray>(
			Unit, DataManager, Pred, false, KbsAlgo::EAdjacencyMode::OrthogonalOnly);
		if (FTacCoordinates FlankCoord = FFlankCellDefinitions::GetAvailableFlankCell(Metadata.Coords, Metadata.Team);
			FlankCoord.IsValidCell())
		{
			if (!DataManager->IsCellOccupied(FlankCoord))
				Result.Add(FlankCoord);
		}
	}
	else
	{
		for (const FIntPoint& Off : FGridConstants::OrthogonalOffsets)
		{
			FTacCoordinates Candidate(Metadata.Coords.Row + Off.X, Metadata.Coords.Col + Off.Y, TargetLayer);
			if (!Candidate.IsValidCell()) continue;
			if (Pred(Unit, DataManager->GetUnit(Candidate), Candidate))
				Result.Add(Candidate);
		}
		FTacCoordinates SamePos(Metadata.Coords.Row, Metadata.Coords.Col, TargetLayer);
		if (SamePos.IsValidCell() && Pred(Unit, DataManager->GetUnit(SamePos), SamePos))
			Result.Add(SamePos);
	}
	return Result;
}

TArray<FTacCoordinates> UTacGridTargetingService::GetSingleCellAnyToAny(AUnit* Unit,
                                                                          const FTargetingDescriptor& Desc) const
{
	const ETacGridLayer UnitLayer = Unit->GetGridMetadata().Coords.Layer;
	const ETacGridLayer TargetLayer = (Desc.MovementLayer == EMovementLayer::CrossLayer)
		? (UnitLayer == ETacGridLayer::Ground ? ETacGridLayer::Air : ETacGridLayer::Ground)
		: (Desc.MovementLayer == EMovementLayer::Air ? ETacGridLayer::Air : ETacGridLayer::Ground);
	auto Pred = (TargetLayer == ETacGridLayer::Air) ? AirMovePredicate : MovePredicateFactory(false);
	TArray<FTacCoordinates> Result;
	DataManager->FilterCells(Unit, TargetLayer, Pred, Result);
	return Result;
}

TArray<FTacCoordinates> UTacGridTargetingService::GetSingleCellLinear(AUnit* Unit,
                                                                        const FTargetingDescriptor& Desc) const
{
	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	const ETacGridLayer UnitLayer = Metadata.Coords.Layer;
	const ETacGridLayer TargetLayer = (Desc.MovementLayer == EMovementLayer::CrossLayer)
		? (UnitLayer == ETacGridLayer::Ground ? ETacGridLayer::Air : ETacGridLayer::Ground)
		: (Desc.MovementLayer == EMovementLayer::Air ? ETacGridLayer::Air : ETacGridLayer::Ground);
	auto Pred = (TargetLayer == ETacGridLayer::Air) ? AirMovePredicate : MovePredicateFactory(false);

	TArray<FTacCoordinates> Result;
	const bool bRowAxis = (Metadata.Orientation == EUnitOrientation::GridTop ||
	                       Metadata.Orientation == EUnitOrientation::GridBottom);
	for (const FIntPoint& Off : bRowAxis ? TArray<FIntPoint>{{-1,0},{1,0}} : TArray<FIntPoint>{{0,-1},{0,1}})
	{
		FTacCoordinates Candidate(Metadata.Coords.Row + Off.X, Metadata.Coords.Col + Off.Y, TargetLayer);
		if (!Candidate.IsValidCell()) continue;
		if (Pred(Unit, DataManager->GetUnit(Candidate), Candidate))
			Result.Add(Candidate);
	}
	if (TargetLayer == UnitLayer)
	{
		if (FTacCoordinates FlankCoord = FFlankCellDefinitions::GetAvailableFlankCell(Metadata.Coords, Metadata.Team);
			FlankCoord.IsValidCell() && !DataManager->IsCellOccupied(FlankCoord))
			Result.Add(FlankCoord);
	}
	else
	{
		FTacCoordinates SamePos(Metadata.Coords.Row, Metadata.Coords.Col, TargetLayer);
		if (SamePos.IsValidCell() && Pred(Unit, DataManager->GetUnit(SamePos), SamePos))
			Result.Add(SamePos);
	}
	return Result;
}

TArray<FTacCoordinates> UTacGridTargetingService::GetMultiCellOrthogonal(AUnit* Unit,
                                                                          const FTargetingDescriptor& Desc) const
{
	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	const FTacCoordinates Primary = Metadata.Coords;
	const FTacCoordinates Extra = Metadata.ExtraCell;
	const ETacGridLayer UnitLayer = Primary.Layer;
	const ETacGridLayer TargetLayer = (Desc.MovementLayer == EMovementLayer::CrossLayer)
		? (UnitLayer == ETacGridLayer::Ground ? ETacGridLayer::Air : ETacGridLayer::Ground)
		: (Desc.MovementLayer == EMovementLayer::Air ? ETacGridLayer::Air : ETacGridLayer::Ground);

	auto IsBlockedByOther = [&](const FTacCoordinates& Cell) -> bool
	{
		if (Cell == Primary || Cell == Extra) return false;
		return DataManager->IsCellOccupied(Cell);
	};

	TSet<FTacCoordinates> Candidates;
	for (const FIntPoint& Offset : FGridConstants::OrthogonalOffsets)
	{
		const FTacCoordinates NewPrimary(Primary.Row + Offset.X, Primary.Col + Offset.Y, TargetLayer);
		const FTacCoordinates NewExtra(Extra.Row + Offset.X, Extra.Col + Offset.Y, TargetLayer);

		if (!NewPrimary.IsValidCell() || !NewExtra.IsValidCell()) continue;
		if (NewPrimary.IsFlankCell() || NewExtra.IsFlankCell()) continue;
		if (IsBlockedByOther(NewPrimary) || IsBlockedByOther(NewExtra)) continue;

		if (NewPrimary == Extra || NewExtra == Primary)
			Candidates.Add(NewPrimary);
		else
		{
			Candidates.Add(NewPrimary);
			Candidates.Add(NewExtra);
		}
	}

	TArray<FTacCoordinates> Result;
	Algo::Copy(Candidates, Result);
	if (TargetLayer == UnitLayer)
	{
		if (FTacCoordinates FlankCoord = FFlankCellDefinitions::GetAvailableFlankCell(Primary, Metadata.Team);
			FlankCoord.IsValidCell() && !DataManager->IsCellOccupied(FlankCoord))
			Result.Add(FlankCoord);
	}
	else
	{
		const FTacCoordinates SamePrimary(Primary.Row, Primary.Col, TargetLayer);
		const FTacCoordinates SameExtra(Extra.Row, Extra.Col, TargetLayer);
		if (SamePrimary.IsValidCell() && SameExtra.IsValidCell() &&
			!SamePrimary.IsFlankCell() && !SameExtra.IsFlankCell() &&
			!IsBlockedByOther(SamePrimary) && !IsBlockedByOther(SameExtra))
		{
			Result.Add(SamePrimary);
			Result.Add(SameExtra);
		}
	}
	return Result;
}

TArray<FTacCoordinates> UTacGridTargetingService::GetMultiCellAnyToAny(AUnit* Unit,
                                                                         const FTargetingDescriptor& Desc) const
{
	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	const FTacCoordinates Primary = Metadata.Coords;
	const FTacCoordinates Extra = Metadata.ExtraCell;
	const ETacGridLayer UnitLayer = Primary.Layer;
	const ETacGridLayer TargetLayer = (Desc.MovementLayer == EMovementLayer::CrossLayer)
		? (UnitLayer == ETacGridLayer::Ground ? ETacGridLayer::Air : ETacGridLayer::Ground)
		: (Desc.MovementLayer == EMovementLayer::Air ? ETacGridLayer::Air : ETacGridLayer::Ground);
	const FIntPoint ExtraOffset(Extra.Row - Primary.Row, Extra.Col - Primary.Col);

	TArray<FTacCoordinates> Result;
	QueryPredicates::FCellFilterPredicate Pred = [&](const AUnit* /*Source*/, const AUnit* /*Target*/, const FTacCoordinates& NewPrimary) -> bool
	{
		const FTacCoordinates NewExtra(NewPrimary.Row + ExtraOffset.X, NewPrimary.Col + ExtraOffset.Y, TargetLayer);
		if (!NewExtra.IsValidCell() || NewPrimary.IsFlankCell() || NewExtra.IsFlankCell()) return false;
		auto IsBlockedByOther = [&](const FTacCoordinates& C) -> bool
		{
			if (C == Primary || C == Extra) return false;
			return DataManager->IsCellOccupied(C);
		};
		return !IsBlockedByOther(NewPrimary) && !IsBlockedByOther(NewExtra);
	};
	DataManager->FilterCells(Unit, TargetLayer, Pred, Result);
	return Result;
}

TArray<FTacCoordinates> UTacGridTargetingService::GetMultiCellLinear(AUnit* Unit,
                                                                      const FTargetingDescriptor& Desc) const
{
	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	const FTacCoordinates Primary = Metadata.Coords;
	const FTacCoordinates Extra = Metadata.ExtraCell;
	const ETacGridLayer UnitLayer = Primary.Layer;
	const ETacGridLayer TargetLayer = (Desc.MovementLayer == EMovementLayer::CrossLayer)
		? (UnitLayer == ETacGridLayer::Ground ? ETacGridLayer::Air : ETacGridLayer::Ground)
		: (Desc.MovementLayer == EMovementLayer::Air ? ETacGridLayer::Air : ETacGridLayer::Ground);

	auto IsBlockedByOther = [&](const FTacCoordinates& C) -> bool
	{
		if (C == Primary || C == Extra) return false;
		return DataManager->IsCellOccupied(C);
	};

	const bool bRowAxis = (Metadata.Orientation == EUnitOrientation::GridTop ||
	                       Metadata.Orientation == EUnitOrientation::GridBottom);

	TArray<FTacCoordinates> Result;
	for (const FIntPoint& Off : bRowAxis ? TArray<FIntPoint>{{-1,0},{1,0}} : TArray<FIntPoint>{{0,-1},{0,1}})
	{
		const FTacCoordinates NewPrimary(Primary.Row + Off.X, Primary.Col + Off.Y, TargetLayer);
		const FTacCoordinates NewExtra(Extra.Row + Off.X, Extra.Col + Off.Y, TargetLayer);

		if (!NewPrimary.IsValidCell() || !NewExtra.IsValidCell()) continue;
		if (NewPrimary.IsFlankCell() || NewExtra.IsFlankCell()) continue;
		if (IsBlockedByOther(NewPrimary) || IsBlockedByOther(NewExtra)) continue;

		if (NewPrimary == Extra || NewExtra == Primary)
			Result.Add(NewPrimary);
		else
		{
			Result.Add(NewPrimary);
			Result.Add(NewExtra);
		}
	}
	if (TargetLayer == UnitLayer)
	{
		if (FTacCoordinates FlankCoord = FFlankCellDefinitions::GetAvailableFlankCell(Primary, Metadata.Team);
			FlankCoord.IsValidCell() && !DataManager->IsCellOccupied(FlankCoord))
			Result.Add(FlankCoord);
	}
	else
	{
		const FTacCoordinates SamePrimary(Primary.Row, Primary.Col, TargetLayer);
		const FTacCoordinates SameExtra(Extra.Row, Extra.Col, TargetLayer);
		if (SamePrimary.IsValidCell() && SameExtra.IsValidCell() &&
			!SamePrimary.IsFlankCell() && !SameExtra.IsFlankCell() &&
			!IsBlockedByOther(SamePrimary) && !IsBlockedByOther(SameExtra))
		{
			Result.Add(SamePrimary);
			Result.Add(SameExtra);
		}
	}
	return Result;
}

bool UTacGridTargetingService::TestMovementCell(AUnit* Unit, FTacCoordinates Cell,
                                                const FTargetingDescriptor& Desc) const
{
	return GetValidMovementCells(Unit, Desc).Contains(Cell);
}

// === Private helpers — utilities ===

bool UTacGridTargetingService::CheckUnitAndData(AUnit* Unit) const
{
	check(Unit);
	return Unit->GetGridMetadata().IsValid();
}

bool UTacGridTargetingService::TestCell(AUnit* SourceUnit, FTacCoordinates Cell,
                                        QueryPredicates::FCellFilterPredicate Predicate) const
{
	AUnit* ClickedUnit = DataManager->GetUnit(Cell);
	return Predicate(SourceUnit, ClickedUnit, Cell);
}

bool UTacGridTargetingService::TestCell(FTacCoordinates Cell, QueryPredicates::FCorpseFilterPredicate Predicate) const
{
	return Predicate(Cell, DataManager->IsCellOccupied(Cell), DataManager->GetTopCorpse(Cell),
	                 DataManager->CorpsesNum(Cell));
}

FResolvedTargets UTacGridTargetingService::BuildTargetsFromCells(FTacCoordinates ClickedCell,
                                                                 const TArray<FTacCoordinates>& QueriedCells) const
{
	FResolvedTargets Result;
	AUnit* ClickedUnit = DataManager->GetUnit(ClickedCell);
	if (ClickedUnit && QueriedCells.Contains(ClickedCell))
		Result.ClickedTarget = ClickedUnit;
	else
		Result.ClickedTarget = nullptr;
	Result.ClickedCorpse = DataManager->GetTopCorpse(ClickedCell);
	Result.bWasCellEmpty = (ClickedUnit == nullptr);
	Result.SecondaryTargets = KbsAlgo::ExtractUnits<TArray>(QueriedCells, DataManager);
	if (Result.ClickedTarget) Result.SecondaryTargets.RemoveSingle(Result.ClickedTarget);
	if (Result.ClickedTarget)
		UE_LOG(LogTacGrid, Log, TEXT("ResolveTargets: clicked [%d,%d] -> primary=%s secondary=%d"),
	       ClickedCell.Row, ClickedCell.Col, *Result.ClickedTarget->GetLogName(),
	       Result.SecondaryTargets.Num());
	return Result;
}

FResolvedTargets UTacGridTargetingService::BuildTargetFromCell(FTacCoordinates ClickedCell,
                                                               bool bHasPassedPredicate) const
{
	FResolvedTargets Result;
	if (bHasPassedPredicate)
	{
		Result.ClickedTarget = DataManager->GetUnit(ClickedCell);
		Result.ClickedCorpse = DataManager->GetTopCorpse(ClickedCell);
	}
	Result.bWasCellEmpty = (DataManager->GetUnit(ClickedCell) == nullptr);
	if (Result.ClickedTarget)
		UE_LOG(LogTacGrid, Log, TEXT("ResolveTargets: clicked [%d,%d] -> primary=%s"),
	       ClickedCell.Row, ClickedCell.Col, *Result.ClickedTarget->GetLogName());
	return Result;
}

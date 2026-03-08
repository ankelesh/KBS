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
#include "GameplayTypes/FlankCellDefinitions.h"
#include "GameplayTypes/CombatTypes.h"


namespace
{
	struct CorpseTargetingFilter
	{
		bool bIsAllowingBlocked;
		EAffiliationFilter Filter;
	};

	constexpr CorpseTargetingFilter CorpseTargetBounds(ETargetReach Reach)
	{
		switch (Reach)
		{
		case ETargetReach::AnyCorpse: return {true, EAffiliationFilter::Any};
		case ETargetReach::AnyNonBlockedCorpse: return {false, EAffiliationFilter::Any};
		case ETargetReach::EnemyCorpse: return {true, EAffiliationFilter::Enemy};
		case ETargetReach::EnemyNonBlockedCorpse: return {false, EAffiliationFilter::Enemy};
		case ETargetReach::FriendlyCorpse: return {true, EAffiliationFilter::Friendly};
		case ETargetReach::FriendlyNonBlockedCorpse: return {false, EAffiliationFilter::Friendly};
		default:
			checkNoEntry(); // crashes in dev builds; stripped in shipping
			return {false, EAffiliationFilter::Any};
		}
	}

	struct AffiliationTargetingFilter
	{
		EAffiliationFilter Filter;
		bool bAllowEmpty;
		bool bAllowFlank;
	};

	constexpr AffiliationTargetingFilter AffiliationTargetBounds(ETargetReach Reach)
	{
		switch (Reach)
		{
		case ETargetReach::AnyEnemy:
		case ETargetReach::Area:
		case ETargetReach::AllEnemies:          return {EAffiliationFilter::Enemy,    false, true};
		case ETargetReach::AllFriendlies:
		case ETargetReach::AnyFriendly:         return {EAffiliationFilter::Friendly, false, true};
		case ETargetReach::EmptyCellOrFriendly: return {EAffiliationFilter::Friendly, true,  false};
		default:
			checkNoEntry();
			return {EAffiliationFilter::Any, false, false};
		}
	}
}

using namespace TargetingPredicates;

UTacGridTargetingService::UTacGridTargetingService()
{
}

void UTacGridTargetingService::Initialize(UGridDataManager* InDataManager)
{
	DataManager = InDataManager;
	checkf(DataManager, TEXT("TacGridTargetingService: DataManager must be valid after Initialize"));
}

TArray<FTacCoordinates> UTacGridTargetingService::GetValidTargetCells(AUnit* Unit, ETargetReach Reach) const
{
	check(Unit);
	TArray<FTacCoordinates> TargetCells;
	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();

	switch (Reach)
	{
	case ETargetReach::None:
		break;
	case ETargetReach::Self:
		TargetCells.Append(Metadata.GetCells());
		break;
	case ETargetReach::ClosestEnemies:
		GetClosestEnemyCells(Unit, TargetCells);
		break;
	case ETargetReach::AnyEnemy:
	case ETargetReach::Area:
	case ETargetReach::AllEnemies:
	case ETargetReach::AllFriendlies:
	case ETargetReach::AnyFriendly:
	case ETargetReach::EmptyCellOrFriendly:
		{
			auto [Filter, bAllowEmpty, bAllowFlank] = AffiliationTargetBounds(Reach);
			GetCellsByAffiliation(Unit, Filter, TargetCells, bAllowEmpty, bAllowFlank);
			break;
		}
	case ETargetReach::EmptyCell:
		DataManager->FilterCells(Unit, Metadata.Coords.Layer, EmptyNotFlankPredicate, TargetCells);
		break;
	case ETargetReach::Movement:
		GetMovementCells(Unit, TargetCells);
		break;
	case ETargetReach::AnyCorpse:
	case ETargetReach::FriendlyCorpse:
	case ETargetReach::EnemyCorpse:
	case ETargetReach::AnyNonBlockedCorpse:
	case ETargetReach::FriendlyNonBlockedCorpse:
	case ETargetReach::EnemyNonBlockedCorpse:
		{
			auto FilterBound = CorpseTargetBounds(Reach);
			GetCorpseCellsByAffiliation(Unit, FilterBound.Filter, FilterBound.bIsAllowingBlocked, TargetCells);
			break;
		}
	default:
		UE_LOG(LogTacGrid, Warning, TEXT("Unknown target reach type: %d"), (int32)Reach);
	}
	UE_LOG(LogTacGrid, Log, TEXT("GetValidTargetCells: %s Reach=%d -> %d cells"), *Unit->GetLogName(), (int32)Reach,
	       TargetCells.Num());
	return TargetCells;
}

TArray<AUnit*> UTacGridTargetingService::GetValidTargetUnits(AUnit* Unit, ETargetReach Reach,
                                                             bool bIncludeSelf) const
{
	check(Unit);
	auto TargetCells = GetValidTargetCells(Unit, Reach);
	if (!bIncludeSelf)
	{
		TargetCells.Remove(Unit->GetGridMetadata().Coords);
		if (Unit->GetGridMetadata().HasExtraCell())
			TargetCells.Remove(Unit->GetGridMetadata().ExtraCell);
	}
	auto TargetUnits{KbsAlgo::ExtractUnits<TArray>(TargetCells, DataManager)};
	UE_LOG(LogTacGrid, Log, TEXT("GetValidTargetUnits: %s Reach=%d -> %d units"), *Unit->GetLogName(), (int32)Reach,
	       TargetUnits.Num());
	return TargetUnits;
}

FResolvedTargets UTacGridTargetingService::ResolveTargetsFromClick(AUnit* SourceUnit, FTacCoordinates ClickedCell,
                                                                   ETargetReach Reach,
                                                                   const FAreaShape* AreaShape) const
{
	check(SourceUnit);

	TArray<FTacCoordinates> Cells;
	switch (Reach)
	{
	case ETargetReach::AllEnemies:
	case ETargetReach::AllFriendlies:
		Cells = GetValidTargetCells(SourceUnit, Reach);
		return BuildTargetsFromCells(ClickedCell, Cells);
	case ETargetReach::Area:
		if (AreaShape)
			GetCellsInArea(SourceUnit, ClickedCell, EAffiliationFilter::Any, *AreaShape, Cells);
		return BuildTargetsFromCells(ClickedCell, Cells);

	case ETargetReach::AnyCorpse:
	case ETargetReach::FriendlyCorpse:
	case ETargetReach::EnemyCorpse:
	case ETargetReach::AnyNonBlockedCorpse:
	case ETargetReach::FriendlyNonBlockedCorpse:
	case ETargetReach::EnemyNonBlockedCorpse:
		{
			auto [bCanTargetBlocked,Filter] = CorpseTargetBounds(Reach);
			return BuildTargetFromCell(ClickedCell,
			                           TestCell(ClickedCell,
			                                    CorpseAffiliationPredicateFactory(SourceUnit,
				                                    Filter, bCanTargetBlocked)));
		}
	case ETargetReach::ClosestEnemies:
		return BuildTargetFromCell(ClickedCell,
		                           CanTargetClosestCell(SourceUnit, EAffiliationFilter::Enemy, ClickedCell));
	case ETargetReach::AnyEnemy:
	case ETargetReach::AnyFriendly:
	case ETargetReach::EmptyCellOrFriendly:
		{
			auto [Filter, bAllowEmpty, bAllowFlank] = AffiliationTargetBounds(Reach);
			return BuildTargetFromCell(ClickedCell,
			                           TestCell(SourceUnit, ClickedCell,
			                                    AffiliationPredicateFactory(Filter, bAllowEmpty, bAllowFlank)));
		}
	case ETargetReach::EmptyCell:
		return BuildTargetFromCell(ClickedCell, TestCell(SourceUnit, ClickedCell, EmptyNotFlankPredicate));
	default:
		UE_LOG(LogTacGrid, Warning, TEXT("Unknown target reach type: %d"), (int32)Reach);
	}

	return FResolvedTargets::MakeEmpty();
}

void UTacGridTargetingService::GetClosestEnemyCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!CheckUnitAndData(Unit)) return;
	auto EnemyPredicate = AffiliationPredicateFactory(EAffiliationFilter::Enemy, false, true);
	TSet<FTacCoordinates> FoundEnemyCells = KbsAlgo::CollectAdjacentCellsIf<TSet>(
		Unit, DataManager.Get(), EnemyPredicate, true);
	if (FTacCoordinates BlockedCell = FFlankCellDefinitions::GetEntranceBlockedCell(Unit->GetGridMetadata().Coords);
		BlockedCell.IsValidCell())
	{
		FoundEnemyCells.Remove(BlockedCell);
	}
	Algo::Copy(FoundEnemyCells, OutCells);
}

bool UTacGridTargetingService::CanTargetClosestCell(AUnit* SourceUnit, EAffiliationFilter Filter,
                                                    FTacCoordinates Cell) const
{
	if (!CheckUnitAndData(SourceUnit)) return false;
	if (FTacCoordinates BlockedCell = FFlankCellDefinitions::GetEntranceBlockedCell(
			SourceUnit->GetGridMetadata().Coords);
		BlockedCell.IsValidCell())
		if (Cell == BlockedCell)
			return false;
	auto Predicate = AffiliationPredicateFactory(Filter, false, true, false);
	return KbsAlgo::AdjacentContains(SourceUnit, DataManager, Cell, Predicate);
}

bool UTacGridTargetingService::CanSelfTarget(AUnit* Source, FTacCoordinates Coord) const
{
	check(Source);
	if (!Coord.IsValidCell()) return false;
	if (Source->GetGridMetadata().Coords == Coord)
		return true;
	if (Source->GetGridMetadata().ExtraCell == Coord)
		return true;
	return false;
}


void UTacGridTargetingService::GetCellsByAffiliation(AUnit* SourceUnit, EAffiliationFilter Filter,
                                                     TArray<FTacCoordinates>& OutCells, bool bAllowEmpty,
                                                     bool bAllowFlank) const
{
	if (!CheckUnitAndData(SourceUnit)) return;
	auto AffiliationPredicate = AffiliationPredicateFactory(Filter, bAllowEmpty, bAllowFlank);
	DataManager->FilterCells(SourceUnit, ETacGridLayer::Ground, AffiliationPredicate, OutCells);
	DataManager->FilterCells(SourceUnit, ETacGridLayer::Air, AffiliationPredicate, OutCells);
}


void UTacGridTargetingService::GetMovementCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!CheckUnitAndData(Unit)) return;
	if (Unit->GetGridMetadata().Coords.Layer == ETacGridLayer::Air)
	{
		GetAirMoveCells(Unit, OutCells);
	}
	else
	{
		GetGroundMoveCells(Unit, OutCells);
	}
}

bool UTacGridTargetingService::TestMovementCell(AUnit* Unit, FTacCoordinates Cell) const
{
	TArray<FTacCoordinates> OutCells;
	GetMovementCells(Unit, OutCells);
	return OutCells.Contains(Cell);
}

void UTacGridTargetingService::GetCorpseCellsByAffiliation(AUnit* Unit, EAffiliationFilter Filter, bool bAllowBlocked,
                                                           TArray<FTacCoordinates>& OutCells) const
{
	DataManager->FilterCorpseCells(Unit, CorpseAffiliationPredicateFactory(Unit, Filter, bAllowBlocked), OutCells);
}

void UTacGridTargetingService::GetGroundMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!CheckUnitAndData(Unit)) return;
	auto& Metadata = Unit->GetGridMetadata();

	if (Metadata.IsMultiCell() && Metadata.HasExtraCell())
	{
		GetMultiCellGroundMoveCells(Unit, OutCells);
		return;
	}

	TArray<FTacCoordinates> Result = KbsAlgo::CollectAdjacentCellsIf<TArray>(
		Unit, DataManager, MovePredicateFactory(false), false,
		KbsAlgo::EAdjacencyMode::OrthogonalOnly);
	if (FTacCoordinates FlankCoord = FFlankCellDefinitions::GetAvailableFlankCell(Metadata.Coords, Metadata.Team);
		FlankCoord.IsValidCell())
	{
		if (!DataManager->IsCellOccupied(FlankCoord))
			Result.Add(FlankCoord);
	}
	Algo::Copy(Result, OutCells);
}

void UTacGridTargetingService::GetMultiCellGroundMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	const FTacCoordinates Primary = Metadata.Coords;
	const FTacCoordinates Extra = Metadata.ExtraCell;

	TSet<FTacCoordinates> Candidates;
	for (const FIntPoint& Offset : FGridConstants::OrthogonalOffsets)
	{
		const FTacCoordinates NewPrimary(Primary.Row + Offset.X, Primary.Col + Offset.Y, Primary.Layer);
		const FTacCoordinates NewExtra(Extra.Row + Offset.X, Extra.Col + Offset.Y, Extra.Layer);

		if (!NewPrimary.IsValidCell() || !NewExtra.IsValidCell()) continue;
		if (NewPrimary.IsFlankCell() || NewExtra.IsFlankCell()) continue;
		auto IsBlockedByOther = [&](const FTacCoordinates& Cell) -> bool
		{
			if (Cell == Primary || Cell == Extra) return false;
			return DataManager->IsCellOccupied(Cell);
		};
		if (IsBlockedByOther(NewPrimary) || IsBlockedByOther(NewExtra)) continue;

		if (NewPrimary == Extra)
			Candidates.Add(NewPrimary);
		else if (NewExtra == Primary)
			Candidates.Add(NewPrimary);
		else
		{
			Candidates.Add(NewPrimary);
			Candidates.Add(NewExtra);
		}
	}
	Algo::Copy(Candidates, OutCells);

	if (FTacCoordinates FlankCoord = FFlankCellDefinitions::GetAvailableFlankCell(Primary, Metadata.Team);
		FlankCoord.IsValidCell())
	{
		if (!DataManager->IsCellOccupied(FlankCoord))
			OutCells.Add(FlankCoord);
	}
}

void UTacGridTargetingService::GetAirMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!CheckUnitAndData(Unit)) return;
	DataManager->FilterCells(Unit, ETacGridLayer::Air, AirMovePredicate, OutCells);
}


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
                                                                 TArray<FTacCoordinates>& QueriedCells) const
{
	FResolvedTargets Result;
	AUnit* ClickedUnit = DataManager->GetUnit(ClickedCell);
	if (ClickedUnit && QueriedCells.Contains(ClickedCell))
		Result.ClickedTarget = ClickedUnit;
	else
		Result.ClickedTarget = nullptr;
	Result.ClickedCorpse = DataManager->GetTopCorpse(ClickedCell);
	Result.bWasCellEmpty = (ClickedUnit == nullptr);
	QueriedCells.Remove(ClickedCell);
	if (ClickedUnit)
		if (ClickedUnit->GetGridMetadata().HasExtraCell())
			QueriedCells.Remove(ClickedUnit->GetGridMetadata().ExtraCell);
	Result.SecondaryTargets = KbsAlgo::ExtractUnits<TArray>(QueriedCells, DataManager);
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


void UTacGridTargetingService::GetCellsInArea(AUnit* SourceUnit, FTacCoordinates CenterCell, EAffiliationFilter Filter,
                                              const FAreaShape& AreaShape, TArray<FTacCoordinates>& OutCells) const
{
	if (!CheckUnitAndData(SourceUnit)) return;
	auto AffiliationPredicate = AffiliationPredicateFactory(Filter, true, true);
	auto Result = KbsAlgo::CollectCellsInArea<TArray>(SourceUnit, DataManager, AffiliationPredicate, CenterCell,
	                                                  AreaShape);
	Algo::Copy(Result, OutCells);
}

bool UTacGridTargetingService::IsAdjacent(AUnit* Source, const FTacCoordinates& Coords) const
{
	if (Source->GetGridMetadata().HasExtraCell())
		if (Source->GetGridMetadata().ExtraCell.DistanceTo(Coords) == 1) return true;
	return Source->GetGridMetadata().Coords.DistanceTo(Coords) == 1;
}

bool UTacGridTargetingService::HasValidTargetAtCell(AUnit* Source, FTacCoordinates TargetCell, ETargetReach Reach) const
{
	if (!CheckUnitAndData(Source)) return false;
	if (!TargetCell.IsValidCell()) return false;
	switch (Reach)
	{
	case ETargetReach::None:
		return false;
	case ETargetReach::Self:
		return CanSelfTarget(Source, TargetCell);
	case ETargetReach::ClosestEnemies:
		return CanTargetClosestCell(Source, EAffiliationFilter::Enemy, TargetCell);
	case ETargetReach::AnyEnemy:
	case ETargetReach::AllEnemies:
	case ETargetReach::Area:
	case ETargetReach::AnyFriendly:
	case ETargetReach::AllFriendlies:
	case ETargetReach::EmptyCellOrFriendly:
		{
			auto [Filter, bAllowEmpty, bAllowFlank] = AffiliationTargetBounds(Reach);
			return TestCell(Source, TargetCell, AffiliationPredicateFactory(Filter, bAllowEmpty, bAllowFlank));
		}
	case ETargetReach::EmptyCell:
		return TestCell(Source, TargetCell, EmptyNotFlankPredicate);
	case ETargetReach::Movement:
		return TestMovementCell(Source, TargetCell);
	case ETargetReach::AnyCorpse:
	case ETargetReach::FriendlyCorpse:
	case ETargetReach::EnemyCorpse:
	case ETargetReach::AnyNonBlockedCorpse:
	case ETargetReach::FriendlyNonBlockedCorpse:
	case ETargetReach::EnemyNonBlockedCorpse:
		{
			auto [bCanTargetBlocked,Filter] = CorpseTargetBounds(Reach);
			return TestCell(TargetCell, CorpseAffiliationPredicateFactory(Source, Filter, bCanTargetBlocked));
		}
	default:
		UE_LOG(LogTacGrid, Warning, TEXT("Unknown target reach type: %d in HasValidTargetAtCell"), (int32)Reach);
	}
	return false;
}

bool UTacGridTargetingService::HasAnyValidTargets(AUnit* Source, ETargetReach Reach) const
{
	check(Source);
	// Trivial cases
	if (Reach == ETargetReach::None)
	{
		return false;
	}
	if (Reach == ETargetReach::Self)
	{
		return true;
	}

	const ETeamSide TeamSide = Source->GetGridMetadata().Team;
	if (Reach == ETargetReach::Area || Reach == ETargetReach::AnyEnemy || Reach == ETargetReach::AllEnemies)
	{
		UBattleTeam* EnemyTeam = DataManager->GetTeamBySide(
			TeamSide == ETeamSide::Attacker ? ETeamSide::Defender : ETeamSide::Attacker);
		return EnemyTeam->IsAnyUnitAlive() && EnemyTeam->IsAnyUnitOnField();
	}

	if (Reach == ETargetReach::AnyFriendly || Reach == ETargetReach::AllFriendlies)
	{
		if (UBattleTeam* FriendlyTeam = DataManager->GetTeamBySide(TeamSide))
		{
			return (FriendlyTeam->IsOtherUnitAlive(Source) && FriendlyTeam->IsOtherUnitOnField(Source));
		}
		return false;
	}

	const TArray<FTacCoordinates> ValidCells = GetValidTargetCells(Source, Reach);
	return ValidCells.Num() > 0;
}

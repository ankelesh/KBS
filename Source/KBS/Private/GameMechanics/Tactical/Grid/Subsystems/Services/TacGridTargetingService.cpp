// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/FlankCellDefinitions.h"
#include "GameMechanics/Units/LargeUnit.h"
#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameplayTypes/CombatTypes.h"


UTacGridTargetingService::UTacGridTargetingService()
{
}

void UTacGridTargetingService::Initialize(UGridDataManager* InDataManager)
{
	DataManager = InDataManager;
}

TArray<FTacCoordinates> UTacGridTargetingService::GetValidTargetCells(AUnit* Unit, ETargetReach Reach, bool bUseFlankTargeting) const
{
	TArray<FTacCoordinates> TargetCells;
	if (!Unit || !DataManager)
	{
		return TargetCells;
	}

	if (bUseFlankTargeting && Reach == ETargetReach::ClosestEnemies)
	{
		GetFlankTargetCells(Unit, TargetCells);
		return TargetCells;
	}

	FTacCoordinates UnitPos;
	ETacGridLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitPos, UnitLayer))
	{
		return TargetCells;
	}

	switch (Reach)
	{
		case ETargetReach::None:
			break;
		case ETargetReach::Self:
			TargetCells.Add(UnitPos);
			break;
		case ETargetReach::ClosestEnemies:
			GetClosestEnemyCells(Unit, TargetCells);
			break;
		case ETargetReach::AnyEnemy:
		case ETargetReach::Area:
			GetAnyEnemyCells(Unit, TargetCells);
			break;
		case ETargetReach::AllEnemies:
			GetAnyEnemyCells(Unit, TargetCells);
			break;
		case ETargetReach::AnyFriendly:
			GetAllFriendlyCells(Unit, TargetCells);
			break;
		case ETargetReach::AllFriendlies:
			GetAllFriendlyCells(Unit, TargetCells);
			break;
		case ETargetReach::EmptyCell:
			GetEmptyCellsForLayer(UnitLayer, TargetCells);
			break;
		case ETargetReach::EmptyCellOrFriendly:
			GetEmptyCellsOrFriendly(Unit, TargetCells);
			break;
		case ETargetReach::Movement:
			GetMovementCells(Unit, TargetCells);
			break;
		case ETargetReach::AnyCorpse:
			GetAnyCorpseCells(TargetCells);
			break;
		case ETargetReach::FriendlyCorpse:
			GetFriendlyCorpseCells(Unit, TargetCells);
			break;
		case ETargetReach::EnemyCorpse:
			GetEnemyCorpseCells(Unit, TargetCells);
			break;
		case ETargetReach::AnyNonBlockedCorpse:
			GetAnyNonBlockedCorpseCells(Unit, TargetCells);
			break;
		case ETargetReach::FriendlyNonBlockedCorpse:
			GetFriendlyNonBlockedCorpseCells(Unit, TargetCells);
			break;
		case ETargetReach::EnemyNonBlockedCorpse:
			GetEnemyNonBlockedCorpseCells(Unit, TargetCells);
			break;
	}
	return TargetCells;
}

TArray<AUnit*> UTacGridTargetingService::GetValidTargetUnits(AUnit* Unit, ETargetReach Reach, bool bUseFlankTargeting, bool bIncludeDeadUnits) const
{
	TArray<AUnit*> TargetUnits;
	if (!Unit || !DataManager)
	{
		return TargetUnits;
	}

	const TArray<FTacCoordinates> TargetCells = GetValidTargetCells(Unit, Reach, bUseFlankTargeting);
	for (const FTacCoordinates& Cell : TargetCells)
	{
		AUnit* TargetUnit = DataManager->GetUnit(Cell);
		if (TargetUnit)
		{
			if (!bIncludeDeadUnits && TargetUnit->IsDead())
			{
				continue;
			}
			TargetUnits.Add(TargetUnit);
		}
	}
	return TargetUnits;
}

FResolvedTargets UTacGridTargetingService::ResolveTargetsFromClick(AUnit* SourceUnit, FTacCoordinates ClickedCell, ETargetReach Reach, const FAreaShape* AreaShape) const
{
	FResolvedTargets Result;
	if (!SourceUnit || !DataManager)
	{
		return Result;
	}

	AUnit* ClickedUnit = DataManager->GetUnit(ClickedCell);
	AUnit* ClickedCorpse = DataManager->GetTopCorpse(ClickedCell);

	switch (Reach)
	{
		case ETargetReach::AllEnemies:
		case ETargetReach::AllFriendlies:
		{
			TArray<AUnit*> AllTargets = GetValidTargetUnits(SourceUnit, Reach, false, false);
			Result.ClickedTarget = ClickedUnit;
			for (AUnit* Target : AllTargets)
			{
				if (Target != ClickedUnit)
				{
					Result.SecondaryTargets.Add(Target);
				}
			}
			Result.bWasCellEmpty = (ClickedUnit == nullptr);
			break;
		}
		case ETargetReach::Area:
		{
			if (AreaShape)
			{
				TArray<AUnit*> AreaTargets = GetUnitsInArea(ClickedCell, *AreaShape);
				Result.ClickedTarget = ClickedUnit;
				for (AUnit* Target : AreaTargets)
				{
					if (Target != ClickedUnit)
					{
						Result.SecondaryTargets.Add(Target);
					}
				}
			}
			else
			{
				Result.ClickedTarget = ClickedUnit;
			}
			Result.bWasCellEmpty = (ClickedUnit == nullptr);
			break;
		}
		case ETargetReach::AnyCorpse:
		case ETargetReach::FriendlyCorpse:
		case ETargetReach::EnemyCorpse:
		case ETargetReach::AnyNonBlockedCorpse:
		case ETargetReach::FriendlyNonBlockedCorpse:
		case ETargetReach::EnemyNonBlockedCorpse:
		{
			Result.ClickedTarget = ClickedCorpse;
			Result.bWasCellEmpty = (ClickedCorpse == nullptr);
			break;
		}
		case ETargetReach::ClosestEnemies:
		case ETargetReach::AnyEnemy:
		case ETargetReach::AnyFriendly:
		case ETargetReach::EmptyCell:
		case ETargetReach::EmptyCellOrFriendly:
		default:
		{
			Result.ClickedTarget = ClickedUnit;
			Result.bWasCellEmpty = (ClickedUnit == nullptr);
			break;
		}
	}
	return Result;
}

bool UTacGridTargetingService::IsValidTargetCell(AUnit* Unit, const FTacCoordinates& Cell, ETargetReach Reach) const
{
	if (!Unit || !DataManager)
	{
		return false;
	}

	const TArray<FTacCoordinates> ValidCells = GetValidTargetCells(Unit, Reach);
	return ValidCells.Contains(Cell);
}

void UTacGridTargetingService::GetClosestEnemyCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!DataManager)
	{
		return;
	}

	FTacCoordinates UnitPos;
	ETacGridLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitPos, UnitLayer))
	{
		return;
	}

	UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(Unit);
	if (!EnemyTeam)
	{
		return;
	}

	for (const FIntPoint& Offset : FGridConstants::AllAdjacentOffsets)
	{
		const int32 TargetRow = UnitPos.Row + Offset.Y;
		const int32 TargetCol = UnitPos.Col + Offset.X;

		FTacCoordinates TargetCoords(TargetRow, TargetCol, UnitLayer);
		if (!DataManager->IsValidCell(TargetCoords))
		{
			continue;
		}
		AUnit* TargetUnit = DataManager->GetUnit(TargetCoords);

		if (TargetUnit && EnemyTeam->ContainsUnit(TargetUnit))
		{
			AddUnitCellUnique(TargetUnit, OutCells);
		}
	}
}

void UTacGridTargetingService::GetFlankTargetCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!DataManager)
	{
		return;
	}

	FTacCoordinates UnitPos;
	ETacGridLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitPos, UnitLayer))
	{
		return;
	}

	UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(Unit);
	if (!EnemyTeam)
	{
		return;
	}

	TArray<FTacCoordinates> AllFlankTargets;
	const TArray<int32> AdjacentCols = { UnitPos.Col - 1, UnitPos.Col + 1 };

	for (int32 Col : AdjacentCols)
	{
		if (Col < 0 || Col >= FGridConstants::GridSize)
		{
			continue;
		}
		for (int32 Row = 0; Row < FGridConstants::GridSize; ++Row)
		{
			FTacCoordinates TargetCoords(Row, Col, UnitLayer);
			AUnit* TargetUnit = DataManager->GetUnit(TargetCoords);

			if (TargetUnit && EnemyTeam->ContainsUnit(TargetUnit))
			{
				AddUnitCellUnique(TargetUnit, AllFlankTargets);
			}
		}
	}

	TArray<FTacCoordinates> ClosestToCenterCells;
	int32 MinDistToCenter = TNumericLimits<int32>::Max();

	for (const FTacCoordinates& Cell : AllFlankTargets)
	{
		const int32 Dist = FMath::Abs(Cell.Row - FGridConstants::CenterRow);
		if (Dist < MinDistToCenter)
		{
			MinDistToCenter = Dist;
			ClosestToCenterCells.Empty();
			ClosestToCenterCells.Add(Cell);
		}
		else if (Dist == MinDistToCenter)
		{
			ClosestToCenterCells.Add(Cell);
		}
	}

	OutCells.Append(ClosestToCenterCells);
}

void UTacGridTargetingService::GetAnyEnemyCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!DataManager)
	{
		return;
	}

	UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(Unit);
	if (!EnemyTeam)
	{
		return;
	}

	for (AUnit* EnemyUnit : EnemyTeam->GetUnits())
	{
		if (EnemyUnit)
		{
			AddUnitCell(EnemyUnit, OutCells);
		}
	}
}

void UTacGridTargetingService::GetAllFriendlyCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!DataManager)
	{
		return;
	}

	UBattleTeam* FriendlyTeam = DataManager->GetTeamForUnit(Unit);
	if (!FriendlyTeam)
	{
		return;
	}

	for (AUnit* FriendlyUnit : FriendlyTeam->GetUnits())
	{
		if (FriendlyUnit && FriendlyUnit != Unit)
		{
			AddUnitCell(FriendlyUnit, OutCells);
		}
	}
}

void UTacGridTargetingService::GetEmptyCellsOrFriendly(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!DataManager)
	{
		return;
	}

	FTacCoordinates UnitPos;
	ETacGridLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitPos, UnitLayer))
	{
		return;
	}

	GetEmptyCellsForLayer(UnitLayer, OutCells);
	GetAllFriendlyCells(Unit, OutCells);
}

void UTacGridTargetingService::GetMovementCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!Unit || !DataManager)
	{
		return;
	}

	FTacCoordinates UnitPos;
	ETacGridLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitPos, UnitLayer))
	{
		return;
	}

	if (UnitLayer == ETacGridLayer::Air)
	{
		GetAirMoveCells(Unit, OutCells);
	}
	else
	{
		GetAdjacentMoveCells(Unit, OutCells);
		GetFlankMoveCells(Unit, OutCells);
	}
}

void UTacGridTargetingService::GetCorpseCells(AUnit* Unit, ECorpseFilter Filter, TArray<FTacCoordinates>& OutCells) const
{
	if (!DataManager)
	{
		return;
	}

	switch (Filter)
	{
		case ECorpseFilter::Any:
			IterateGroundCells([this](const FTacCoordinates& Coords) {
				return DataManager->HasCorpses(Coords);
			}, OutCells);
			break;

		case ECorpseFilter::Friendly:
		{
			if (!Unit)
			{
				return;
			}
			UBattleTeam* FriendlyTeam = DataManager->GetTeamForUnit(Unit);
			if (!FriendlyTeam)
			{
				return;
			}
			IterateGroundCells([this, FriendlyTeam](const FTacCoordinates& Coords) {
				AUnit* TopCorpse = DataManager->GetTopCorpse(Coords);
				return TopCorpse && FriendlyTeam->ContainsUnit(TopCorpse);
			}, OutCells);
			break;
		}

		case ECorpseFilter::Enemy:
		{
			if (!Unit)
			{
				return;
			}
			UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(Unit);
			if (!EnemyTeam)
			{
				return;
			}
			IterateGroundCells([this, EnemyTeam](const FTacCoordinates& Coords) {
				AUnit* TopCorpse = DataManager->GetTopCorpse(Coords);
				return TopCorpse && EnemyTeam->ContainsUnit(TopCorpse);
			}, OutCells);
			break;
		}

		case ECorpseFilter::AnyNonBlocked:
			IterateGroundCells([this](const FTacCoordinates& Coords) {
				return DataManager->HasCorpses(Coords) && !DataManager->IsCellOccupied(Coords);
			}, OutCells);
			break;

		case ECorpseFilter::FriendlyNonBlocked:
		{
			if (!Unit)
			{
				return;
			}
			UBattleTeam* FriendlyTeam = DataManager->GetTeamForUnit(Unit);
			if (!FriendlyTeam)
			{
				return;
			}
			IterateGroundCells([this, FriendlyTeam](const FTacCoordinates& Coords) {
				AUnit* TopCorpse = DataManager->GetTopCorpse(Coords);
				return TopCorpse && FriendlyTeam->ContainsUnit(TopCorpse) && !DataManager->IsCellOccupied(Coords);
			}, OutCells);
			break;
		}

		case ECorpseFilter::EnemyNonBlocked:
		{
			if (!Unit)
			{
				return;
			}
			UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(Unit);
			if (!EnemyTeam)
			{
				return;
			}
			IterateGroundCells([this, EnemyTeam](const FTacCoordinates& Coords) {
				AUnit* TopCorpse = DataManager->GetTopCorpse(Coords);
				return TopCorpse && EnemyTeam->ContainsUnit(TopCorpse) && !DataManager->IsCellOccupied(Coords);
			}, OutCells);
			break;
		}
	}
}

void UTacGridTargetingService::GetAnyCorpseCells(TArray<FTacCoordinates>& OutCells) const
{
	GetCorpseCells(nullptr, ECorpseFilter::Any, OutCells);
}

void UTacGridTargetingService::GetFriendlyCorpseCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	GetCorpseCells(Unit, ECorpseFilter::Friendly, OutCells);
}

void UTacGridTargetingService::GetEnemyCorpseCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	GetCorpseCells(Unit, ECorpseFilter::Enemy, OutCells);
}

void UTacGridTargetingService::GetAnyNonBlockedCorpseCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	GetCorpseCells(Unit, ECorpseFilter::AnyNonBlocked, OutCells);
}

void UTacGridTargetingService::GetFriendlyNonBlockedCorpseCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	GetCorpseCells(Unit, ECorpseFilter::FriendlyNonBlocked, OutCells);
}

void UTacGridTargetingService::GetEnemyNonBlockedCorpseCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	GetCorpseCells(Unit, ECorpseFilter::EnemyNonBlocked, OutCells);
}

void UTacGridTargetingService::GetAdjacentMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!DataManager)
	{
		return;
	}

	FTacCoordinates UnitPos;
	ETacGridLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitPos, UnitLayer))
	{
		return;
	}

	UBattleTeam* UnitTeam = DataManager->GetTeamForUnit(Unit);

	for (const FIntPoint& Offset : FGridConstants::OrthogonalOffsets)
	{
		const int32 TargetRow = UnitPos.Row + Offset.Y;
		const int32 TargetCol = UnitPos.Col + Offset.X;

		FTacCoordinates TargetCoords(TargetRow, TargetCol, UnitLayer);
		if (!DataManager->IsValidCell(TargetCoords))
		{
			continue;
		}

		if (DataManager->IsFlankCell(TargetCoords))
		{
			UBattleTeam* AttackerTeam = DataManager->GetAttackerTeam();
			if (UnitTeam == AttackerTeam && FFlankCellDefinitions::IsAttackerFlankColumn(TargetCol))
			{
				continue;
			}
			if (UnitTeam != AttackerTeam && FFlankCellDefinitions::IsDefenderFlankColumn(TargetCol))
			{
				continue;
			}
		}

		AUnit* OccupyingUnit = DataManager->GetUnit(TargetCoords);
		if (OccupyingUnit && (!UnitTeam || !UnitTeam->ContainsUnit(OccupyingUnit)))
		{
			continue;
		}

		if (!IsValidMultiCellDestination(Unit, TargetCoords, UnitLayer))
		{
			continue;
		}

		OutCells.Add(TargetCoords);
	}
}

void UTacGridTargetingService::GetFlankMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!DataManager)
	{
		return;
	}

	FTacCoordinates UnitPos;
	ETacGridLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitPos, UnitLayer))
	{
		return;
	}

	UBattleTeam* UnitTeam = DataManager->GetTeamForUnit(Unit);
	if (!UnitTeam)
	{
		return;
	}

	UBattleTeam* AttackerTeam = DataManager->GetAttackerTeam();
	const bool bIsAttacker = (UnitTeam == AttackerTeam);

	TArray<int32> EnemyFlankColumns;
	if (bIsAttacker)
	{
		EnemyFlankColumns = FFlankCellDefinitions::DefenderFlankColumns;
	}
	else
	{
		EnemyFlankColumns = FFlankCellDefinitions::AttackerFlankColumns;
	}

	for (int32 Row = 0; Row < FGridConstants::GridSize; ++Row)
	{
		for (int32 Col : EnemyFlankColumns)
		{
			FTacCoordinates FlankCoords(Row, Col, UnitLayer);
			if (!DataManager->IsValidCell(FlankCoords))
			{
				continue;
			}

			if (!DataManager->IsFlankCell(FlankCoords))
			{
				continue;
			}

			if (CanEnterFlankCell(UnitPos, FlankCoords, UnitTeam))
			{
				AUnit* Occupant = DataManager->GetUnit(FlankCoords);
				if (Occupant && (!UnitTeam || !UnitTeam->ContainsUnit(Occupant)))
				{
					continue;
				}

				if (!IsValidMultiCellDestination(Unit, FlankCoords, UnitLayer))
				{
					continue;
				}

				OutCells.Add(FlankCoords);
			}
		}
	}
}

void UTacGridTargetingService::GetAirMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	if (!DataManager)
	{
		return;
	}

	FTacCoordinates UnitPos;
	ETacGridLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitPos, UnitLayer))
	{
		return;
	}

	UBattleTeam* UnitTeam = DataManager->GetTeamForUnit(Unit);

	for (int32 Row = 0; Row < FGridConstants::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridConstants::GridSize; ++Col)
		{
			FTacCoordinates TargetCoords(Row, Col, UnitLayer);
			if (DataManager->IsRestrictedCell(TargetCoords))
			{
				continue;
			}

			AUnit* OccupyingUnit = DataManager->GetUnit(TargetCoords);
			if (OccupyingUnit && (!UnitTeam || !UnitTeam->ContainsUnit(OccupyingUnit)))
			{
				continue;
			}

			if (!IsValidMultiCellDestination(Unit, TargetCoords, UnitLayer))
			{
				continue;
			}

			OutCells.Add(TargetCoords);
		}
	}
}

void UTacGridTargetingService::GetEmptyCellsForLayer(ETacGridLayer Layer, TArray<FTacCoordinates>& OutCells) const
{
	if (!DataManager)
	{
		return;
	}

	OutCells.Append(DataManager->GetValidPlacementCells(Layer));
}

void UTacGridTargetingService::IterateGroundCells(TFunctionRef<bool(const FTacCoordinates&)> FilterPredicate, TArray<FTacCoordinates>& OutCells) const
{
	if (!DataManager)
	{
		return;
	}

	for (int32 Row = 0; Row < FGridConstants::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridConstants::GridSize; ++Col)
		{
			FTacCoordinates Coords(Row, Col, ETacGridLayer::Ground);
			if (FilterPredicate(Coords))
			{
				OutCells.Add(Coords);
			}
		}
	}
}

bool UTacGridTargetingService::TryGetPrimaryCellForUnit(AUnit* Unit, FTacCoordinates& OutCell) const
{
	if (!Unit || !DataManager)
	{
		return false;
	}

	FTacCoordinates Pos;
	ETacGridLayer Layer;
	if (!DataManager->GetUnitPosition(Unit, Pos, Layer))
	{
		return false;
	}

	if (DataManager->IsMultiCellUnit(Unit))
	{
		const FMultiCellUnitData* MultiCellData = DataManager->GetMultiCellData(Unit);
		if (MultiCellData)
		{
			OutCell = MultiCellData->PrimaryCell;
			return true;
		}
		return false;
	}

	OutCell = Pos;
	return true;
}

bool UTacGridTargetingService::IsValidMultiCellDestination(AUnit* Unit, const FTacCoordinates& TargetCoords, ETacGridLayer Layer) const
{
	if (!Unit || !DataManager || !Unit->IsMultiCell())
	{
		return true;
	}

	bool bTargetIsFlank = DataManager->IsFlankCell(TargetCoords);
	FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(TargetCoords.Row, TargetCoords.Col, bTargetIsFlank, Unit->GetTeamSide());

	FTacCoordinates SecondaryCoords(SecondaryCell.Y, SecondaryCell.X, Layer);
	if (!DataManager->IsValidCell(SecondaryCoords))
	{
		return false;
	}

	AUnit* SecondaryOccupant = DataManager->GetUnit(SecondaryCoords);
	if (SecondaryOccupant && SecondaryOccupant != Unit)
	{
		return false;
	}

	return true;
}

void UTacGridTargetingService::AddUnitCell(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	FTacCoordinates Cell;
	if (TryGetPrimaryCellForUnit(Unit, Cell))
	{
		OutCells.Add(Cell);
	}
}

void UTacGridTargetingService::AddUnitCellUnique(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	FTacCoordinates Cell;
	if (TryGetPrimaryCellForUnit(Unit, Cell))
	{
		OutCells.AddUnique(Cell);
	}
}

bool UTacGridTargetingService::CanEnterFlankCell(const FTacCoordinates& UnitPos, const FTacCoordinates& FlankCell, UBattleTeam* UnitTeam) const
{
	if (!DataManager || !UnitTeam)
	{
		return false;
	}

	const int32 UnitRow = UnitPos.Row;
	const int32 UnitCol = UnitPos.Col;
	const int32 FlankRow = FlankCell.Row;
	const int32 FlankCol = FlankCell.Col;

	UBattleTeam* AttackerTeam = DataManager->GetAttackerTeam();
	const bool bIsAttacker = (UnitTeam == AttackerTeam);

	const bool bIsEnemyFlank = (bIsAttacker && FFlankCellDefinitions::IsDefenderFlankColumn(FlankCol)) ||
	                           (!bIsAttacker && FFlankCellDefinitions::IsAttackerFlankColumn(FlankCol));
	if (!bIsEnemyFlank)
	{
		return false;
	}

	const bool bIsClosestFlank = FFlankCellDefinitions::IsClosestFlankColumn(FlankCol);
	const bool bIsFarFlank = FFlankCellDefinitions::IsFarFlankColumn(FlankCol);

	if (bIsClosestFlank && FFlankCellDefinitions::IsCenterLineCell(UnitRow, UnitCol))
	{
		return IsAdjacentCell(UnitPos, FlankCell);
	}

	if (bIsClosestFlank && IsFlankCell(UnitPos))
	{
		const bool bIsOwnFlank = (bIsAttacker && FFlankCellDefinitions::IsAttackerFlankColumn(UnitCol)) ||
		                         (!bIsAttacker && FFlankCellDefinitions::IsDefenderFlankColumn(UnitCol));
		if (bIsOwnFlank && IsAdjacentCell(UnitPos, FlankCell))
		{
			return true;
		}
	}

	if (bIsFarFlank && IsFlankCell(UnitPos))
	{
		const bool bIsInClosestEnemyFlank = (bIsAttacker && UnitCol == 3) || (!bIsAttacker && UnitCol == 1);
		if (bIsInClosestEnemyFlank && IsAdjacentCell(UnitPos, FlankCell))
		{
			return true;
		}
	}

	if (bIsFarFlank && !IsFlankCell(UnitPos))
	{
		return IsAdjacentCell(UnitPos, FlankCell);
	}

	return false;
}

bool UTacGridTargetingService::IsAdjacentCell(const FTacCoordinates& CellA, const FTacCoordinates& CellB) const
{
	const int32 ManhattanDistance = FMath::Abs(CellA.Col - CellB.Col) + FMath::Abs(CellA.Row - CellB.Row);
	return ManhattanDistance == 1;
}

bool UTacGridTargetingService::IsFlankCell(const FTacCoordinates& Cell) const
{
	if (!DataManager)
	{
		return false;
	}
	return DataManager->IsFlankCell(Cell);
}

TArray<AUnit*> UTacGridTargetingService::GetUnitsInArea(FTacCoordinates CenterCell, const FAreaShape& AreaShape) const
{
	TArray<AUnit*> UnitsInArea;
	if (!DataManager)
	{
		return UnitsInArea;
	}

	TArray<FTacCoordinates> TargetCells;
	for (const FIntPoint& RelativeOffset : AreaShape.RelativeCells)
	{
		const int32 TargetCol = CenterCell.Col + RelativeOffset.X;
		const int32 TargetRow = CenterCell.Row + RelativeOffset.Y;

		FTacCoordinates TargetCoords(TargetRow, TargetCol, CenterCell.Layer);
		if (DataManager->IsValidCell(TargetCoords))
		{
			TargetCells.Add(TargetCoords);
		}
	}

	TArray<AUnit*> UnitsAtLayer = DataManager->GetUnitsInCells(TargetCells, CenterCell.Layer);
	for (AUnit* Unit : UnitsAtLayer)
	{
		if (Unit && !Unit->IsDead())
		{
			UnitsInArea.Add(Unit);
		}
	}

	if (AreaShape.bAffectsAllLayers)
	{
		ETacGridLayer OtherLayer = (CenterCell.Layer == ETacGridLayer::Ground) ? ETacGridLayer::Air : ETacGridLayer::Ground;
		TArray<AUnit*> UnitsAtOtherLayer = DataManager->GetUnitsInCells(TargetCells, OtherLayer);
		for (AUnit* Unit : UnitsAtOtherLayer)
		{
			if (Unit && !Unit->IsDead())
			{
				UnitsInArea.Add(Unit);
			}
		}
	}

	return UnitsInArea;
}

int32 UTacGridTargetingService::CalculateDistance(AUnit* Unit1, AUnit* Unit2) const
{
	if (!Unit1 || !Unit2 || !DataManager)
	{
		return -1;
	}

	FTacCoordinates Pos1, Pos2;
	ETacGridLayer Layer1, Layer2;

	if (!DataManager->GetUnitPosition(Unit1, Pos1, Layer1) ||
	    !DataManager->GetUnitPosition(Unit2, Pos2, Layer2))
	{
		return -1;
	}

	// Chebyshev distance (max of row/col difference) - matches grid adjacency logic
	int32 RowDiff = FMath::Abs(Pos1.Row - Pos2.Row);
	int32 ColDiff = FMath::Abs(Pos1.Col - Pos2.Col);
	return FMath::Max(RowDiff, ColDiff);
}

UWeapon* UTacGridTargetingService::SelectWeapon(AUnit* Attacker, AUnit* Target) const
{
	if (!Attacker || !Target || !DataManager)
	{
		return nullptr;
	}

	const TArray<TObjectPtr<UWeapon>>& Weapons = Attacker->GetWeapons();
	if (Weapons.Num() == 0)
	{
		return nullptr;
	}

	int32 Distance = CalculateDistance(Attacker, Target);
	if (Distance < 0)
	{
		return nullptr;
	}

	UBattleTeam* AttackerTeam = DataManager->GetTeamForUnit(Attacker);
	UBattleTeam* TargetTeam = DataManager->GetTeamForUnit(Target);
	bool bIsFriendlyTarget = (AttackerTeam == TargetTeam);

	TArray<UWeapon*> ValidWeapons;
	for (UWeapon* Weapon : Weapons)
	{
		if (!Weapon)
		{
			continue;
		}

		const FWeaponStats& Stats = Weapon->GetStats();
		ETargetReach WeaponReach = Stats.TargetReach;

		// Check if weapon can target this type (friendly vs hostile)
		bool bCanTargetType = false;
		if (bIsFriendlyTarget)
		{
			bCanTargetType = FDamageCalculation::IsFriendlyReach(WeaponReach);
		}
		else
		{
			// Hostile targeting
			if (WeaponReach == ETargetReach::ClosestEnemies && Distance == 1)
			{
				bCanTargetType = true;
			}
			else if (WeaponReach == ETargetReach::AnyEnemy ||
			         WeaponReach == ETargetReach::AllEnemies ||
			         WeaponReach == ETargetReach::Area)
			{
				bCanTargetType = true;
			}
		}

		if (bCanTargetType)
		{
			ValidWeapons.Add(Weapon);
		}
	}

	if (ValidWeapons.Num() == 0)
	{
		return nullptr;
	}

	// Pick weapon with highest damage output
	UWeapon* BestWeapon = nullptr;
	int32 BestDamage = -1;
	for (UWeapon* Weapon : ValidWeapons)
	{
		FDamageResult DamageResult = FDamageCalculation::CalculateDamage(Attacker, Weapon, Target);
		if (DamageResult.Damage > BestDamage)
		{
			BestDamage = DamageResult.Damage;
			BestWeapon = Weapon;
		}
	}

	return BestWeapon;
}

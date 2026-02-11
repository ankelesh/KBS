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
	checkf(DataManager, TEXT("TacGridTargetingService: DataManager must be valid after Initialize"));
}

TArray<FTacCoordinates> UTacGridTargetingService::GetValidTargetCells(AUnit* Unit, ETargetReach Reach, bool bUseFlankTargeting) const
{
	check(Unit);

	TArray<FTacCoordinates> TargetCells;

	if (bUseFlankTargeting && Reach == ETargetReach::ClosestEnemies)
	{
		GetFlankTargetCells(Unit, TargetCells);
		return TargetCells;
	}

	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	check(Metadata.IsValid());

	switch (Reach)
	{
		case ETargetReach::None:
			break;
		case ETargetReach::Self:
			TargetCells.Add(Metadata.Coords);
			break;
		case ETargetReach::ClosestEnemies:
			GetClosestEnemyCells(Unit, TargetCells);
			break;
		case ETargetReach::AnyEnemy:
		case ETargetReach::Area:
			GetUnitCellsByAffiliation(Unit, EAffiliationFilter::Enemy, TargetCells);
			break;
		case ETargetReach::AllEnemies:
			GetUnitCellsByAffiliation(Unit, EAffiliationFilter::Enemy, TargetCells);
			break;
		case ETargetReach::AnyFriendly:
			GetUnitCellsByAffiliation(Unit, EAffiliationFilter::Friendly, TargetCells);
			break;
		case ETargetReach::AllFriendlies:
			GetUnitCellsByAffiliation(Unit, EAffiliationFilter::Friendly, TargetCells);
			break;
		case ETargetReach::EmptyCell:
			GetEmptyCellsForLayer(Metadata.Coords.Layer, TargetCells);
			break;
		case ETargetReach::EmptyCellOrFriendly:
			GetEmptyCellsOrFriendly(Unit, TargetCells);
			break;
		case ETargetReach::Movement:
			GetMovementCells(Unit, TargetCells);
			break;
		case ETargetReach::AnyCorpse:
			GetCorpseCells(nullptr, ECorpseFilter::Any, TargetCells);
			break;
		case ETargetReach::FriendlyCorpse:
			GetCorpseCells(Unit, ECorpseFilter::Friendly, TargetCells);
			break;
		case ETargetReach::EnemyCorpse:
			GetCorpseCells(Unit, ECorpseFilter::Enemy, TargetCells);
			break;
		case ETargetReach::AnyNonBlockedCorpse:
			GetCorpseCells(Unit, ECorpseFilter::AnyNonBlocked, TargetCells);
			break;
		case ETargetReach::FriendlyNonBlockedCorpse:
			GetCorpseCells(Unit, ECorpseFilter::FriendlyNonBlocked, TargetCells);
			break;
		case ETargetReach::EnemyNonBlockedCorpse:
			GetCorpseCells(Unit, ECorpseFilter::EnemyNonBlocked, TargetCells);
			break;
	}
	return TargetCells;
}

TArray<AUnit*> UTacGridTargetingService::GetValidTargetUnits(AUnit* Unit, ETargetReach Reach, bool bUseFlankTargeting, bool bIncludeDeadUnits) const
{
	check(Unit);

	TArray<AUnit*> TargetUnits;
	const TArray<FTacCoordinates> TargetCells = GetValidTargetCells(Unit, Reach, bUseFlankTargeting);
	for (const FTacCoordinates& Cell : TargetCells)
	{
		AUnit* TargetUnit = DataManager->GetUnit(Cell);
		if (TargetUnit)
		{
			if (!bIncludeDeadUnits && TargetUnit->GetStats().Health.IsDead())
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
	check(SourceUnit);

	FResolvedTargets Result;
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
	check(Unit);

	const TArray<FTacCoordinates> ValidCells = GetValidTargetCells(Unit, Reach);
	return ValidCells.Contains(Cell);
}

void UTacGridTargetingService::GetClosestEnemyCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	check(Unit);

	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	check(Metadata.IsValid());

	for (const FIntPoint& Offset : FGridConstants::AllAdjacentOffsets)
	{
		FTacCoordinates TargetCoords(Metadata.Coords.Row + Offset.Y, Metadata.Coords.Col + Offset.X, Metadata.Coords.Layer);

		if (!DataManager->IsValidCell(TargetCoords))
		{
			continue;
		}

		if (AUnit* TargetUnit = DataManager->GetUnit(TargetCoords); TargetUnit && Metadata.IsEnemy(TargetUnit->GetGridMetadata()))
		{
			AddUnitCellUnique(TargetUnit, OutCells);
		}
	}
}

void UTacGridTargetingService::GetFlankTargetCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	check(Unit);

	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	check(Metadata.IsValid());

	TArray<FTacCoordinates> AllFlankTargets;
	const TArray<int32> AdjacentCols = { Metadata.Coords.Col - 1, Metadata.Coords.Col + 1 };

	for (int32 Col : AdjacentCols)
	{
		if (Col < 0 || Col >= FGridConstants::GridSize)
		{
			continue;
		}
		for (int32 Row = 0; Row < FGridConstants::GridSize; ++Row)
		{
			FTacCoordinates TargetCoords(Row, Col, Metadata.Coords.Layer);

			if (AUnit* TargetUnit = DataManager->GetUnit(TargetCoords); TargetUnit && Metadata.IsEnemy(TargetUnit->GetGridMetadata()))
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

void UTacGridTargetingService::GetUnitCellsByAffiliation(AUnit* SourceUnit, EAffiliationFilter Filter, TArray<FTacCoordinates>& OutCells) const
{
	check(SourceUnit);

	const FUnitGridMetadata& SourceMetadata = SourceUnit->GetGridMetadata();
	check(SourceMetadata.IsValid());

	UBattleTeam* FriendlyTeam = DataManager->GetTeamForUnit(SourceUnit);
	UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(SourceUnit);

	if (Filter == EAffiliationFilter::Any || Filter == EAffiliationFilter::Friendly)
	{
		if (FriendlyTeam)
		{
			for (AUnit* Unit : FriendlyTeam->GetUnits())
			{
				if (Unit && Unit != SourceUnit)
				{
					AddUnitCell(Unit, OutCells);
				}
			}
		}
	}

	if (Filter == EAffiliationFilter::Any || Filter == EAffiliationFilter::Enemy)
	{
		if (EnemyTeam)
		{
			for (AUnit* Unit : EnemyTeam->GetUnits())
			{
				if (Unit)
				{
					AddUnitCell(Unit, OutCells);
				}
			}
		}
	}
}

void UTacGridTargetingService::GetEmptyCellsOrFriendly(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	check(Unit);

	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	check(Metadata.IsValid());

	GetEmptyCellsForLayer(Metadata.Coords.Layer, OutCells);
	GetUnitCellsByAffiliation(Unit, EAffiliationFilter::Friendly, OutCells);
}

void UTacGridTargetingService::GetMovementCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	check(Unit);

	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	check(Metadata.IsValid());

	if (Metadata.Coords.Layer == ETacGridLayer::Air)
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
	const bool bRequireNonBlocked = (Filter == ECorpseFilter::AnyNonBlocked ||
	                                  Filter == ECorpseFilter::FriendlyNonBlocked ||
	                                  Filter == ECorpseFilter::EnemyNonBlocked);

	const bool bRequireFriendly = (Filter == ECorpseFilter::Friendly || Filter == ECorpseFilter::FriendlyNonBlocked);
	const bool bRequireEnemy = (Filter == ECorpseFilter::Enemy || Filter == ECorpseFilter::EnemyNonBlocked);
	const bool bAnyAffiliation = (Filter == ECorpseFilter::Any || Filter == ECorpseFilter::AnyNonBlocked);

	if (!bAnyAffiliation)
	{
		check(Unit);
	}

	const FUnitGridMetadata* SourceMetadata = nullptr;
	if (Unit)
	{
		SourceMetadata = &Unit->GetGridMetadata();
		check(SourceMetadata->IsValid());
	}

	IterateGroundCells([this, SourceMetadata, bRequireNonBlocked, bRequireFriendly, bRequireEnemy, bAnyAffiliation](const FTacCoordinates& Coords) {
		if (bRequireNonBlocked && DataManager->IsCellOccupied(Coords))
		{
			return false;
		}

		AUnit* TopCorpse = DataManager->GetTopCorpse(Coords);
		if (!TopCorpse)
		{
			return DataManager->HasCorpses(Coords) && bAnyAffiliation;
		}

		if (bAnyAffiliation)
		{
			return true;
		}

		if (bRequireFriendly)
		{
			return SourceMetadata->IsAlly(TopCorpse->GetGridMetadata());
		}

		if (bRequireEnemy)
		{
			return SourceMetadata->IsEnemy(TopCorpse->GetGridMetadata());
		}

		return false;
	}, OutCells);
}

void UTacGridTargetingService::GetAdjacentMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	check(Unit);

	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	check(Metadata.IsValid());

	UBattleTeam* UnitTeam = DataManager->GetTeamForUnit(Unit);
	UBattleTeam* AttackerTeam = DataManager->GetAttackerTeam();
	const bool bIsAttacker = (UnitTeam == AttackerTeam);

	for (const FIntPoint& Offset : FGridConstants::OrthogonalOffsets)
	{
		FTacCoordinates TargetCoords(Metadata.Coords.Row + Offset.Y, Metadata.Coords.Col + Offset.X, Metadata.Coords.Layer);

		if (!DataManager->IsValidCell(TargetCoords))
		{
			continue;
		}

		if (DataManager->IsFlankCell(TargetCoords))
		{
			if (bIsAttacker && FFlankCellDefinitions::IsAttackerFlankColumn(TargetCoords.Col))
			{
				continue;
			}
			if (!bIsAttacker && FFlankCellDefinitions::IsDefenderFlankColumn(TargetCoords.Col))
			{
				continue;
			}
		}

		if (!IsValidMoveDestination(Unit, TargetCoords))
		{
			continue;
		}

		OutCells.Add(TargetCoords);
	}
}

void UTacGridTargetingService::GetFlankMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	check(Unit);

	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	check(Metadata.IsValid());

	UBattleTeam* UnitTeam = DataManager->GetTeamForUnit(Unit);
	UBattleTeam* AttackerTeam = DataManager->GetAttackerTeam();
	const bool bIsAttacker = (UnitTeam == AttackerTeam);

	const TArray<int32>& EnemyFlankColumns = bIsAttacker ?
		FFlankCellDefinitions::DefenderFlankColumns :
		FFlankCellDefinitions::AttackerFlankColumns;

	for (int32 Row = 0; Row < FGridConstants::GridSize; ++Row)
	{
		for (int32 Col : EnemyFlankColumns)
		{
			FTacCoordinates FlankCoords(Row, Col, Metadata.Coords.Layer);

			if (!DataManager->IsValidCell(FlankCoords) || !DataManager->IsFlankCell(FlankCoords))
			{
				continue;
			}

			if (CanEnterFlankCell(Metadata.Coords, FlankCoords, UnitTeam) && IsValidMoveDestination(Unit, FlankCoords))
			{
				OutCells.Add(FlankCoords);
			}
		}
	}
}

void UTacGridTargetingService::GetAirMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const
{
	check(Unit);

	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	check(Metadata.IsValid());

	for (int32 Row = 0; Row < FGridConstants::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridConstants::GridSize; ++Col)
		{
			FTacCoordinates TargetCoords(Row, Col, Metadata.Coords.Layer);

			if (DataManager->IsRestrictedCell(TargetCoords))
			{
				continue;
			}

			if (IsValidMoveDestination(Unit, TargetCoords))
			{
				OutCells.Add(TargetCoords);
			}
		}
	}
}

void UTacGridTargetingService::GetEmptyCellsForLayer(ETacGridLayer Layer, TArray<FTacCoordinates>& OutCells) const
{
	OutCells.Append(DataManager->GetValidPlacementCells(Layer));
}

void UTacGridTargetingService::IterateGroundCells(TFunctionRef<bool(const FTacCoordinates&)> FilterPredicate, TArray<FTacCoordinates>& OutCells) const
{
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
	if (!Unit)
	{
		return false;
	}

	const FUnitGridMetadata& Metadata = Unit->GetGridMetadata();
	if (!Metadata.IsValid())
	{
		return false;
	}

	if (DataManager->IsMultiCellUnit(Unit))
	{
		if (const FMultiCellUnitData* MultiCellData = DataManager->GetMultiCellData(Unit))
		{
			OutCell = MultiCellData->PrimaryCell;
			return true;
		}
		return false;
	}

	OutCell = Metadata.Coords;
	return true;
}

bool UTacGridTargetingService::IsValidMultiCellDestination(AUnit* Unit, const FTacCoordinates& TargetCoords, ETacGridLayer Layer) const
{
	if (!Unit || !Unit->IsMultiCell())
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

	if (AUnit* SecondaryOccupant = DataManager->GetUnit(SecondaryCoords); SecondaryOccupant && SecondaryOccupant != Unit)
	{
		return false;
	}

	return true;
}

bool UTacGridTargetingService::IsValidMoveDestination(AUnit* MovingUnit, const FTacCoordinates& TargetCoords) const
{
	check(MovingUnit);

	if (AUnit* Occupant = DataManager->GetUnit(TargetCoords))
	{
		if (!MovingUnit->GetGridMetadata().IsAlly(Occupant->GetGridMetadata()))
		{
			return false;
		}
	}

	return IsValidMultiCellDestination(MovingUnit, TargetCoords, TargetCoords.Layer);
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
	check(UnitTeam);

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
		if (Unit && !Unit->GetStats().Health.IsDead())
		{
			UnitsInArea.Add(Unit);
		}
	}

	if (AreaShape.ShapeLayering == EShapeLayering::BothLayerArea)
	{
		ETacGridLayer OtherLayer = (CenterCell.Layer == ETacGridLayer::Ground) ? ETacGridLayer::Air : ETacGridLayer::Ground;
		TArray<AUnit*> UnitsAtOtherLayer = DataManager->GetUnitsInCells(TargetCells, OtherLayer);
		for (AUnit* Unit : UnitsAtOtherLayer)
		{
			if (Unit && !Unit->GetStats().Health.IsDead())
			{
				UnitsInArea.Add(Unit);
			}
		}
	}

	return UnitsInArea;
}

int32 UTacGridTargetingService::CalculateDistance(AUnit* Unit1, AUnit* Unit2) const
{
	check(Unit1 && Unit2);

	const FUnitGridMetadata& Metadata1 = Unit1->GetGridMetadata();
	const FUnitGridMetadata& Metadata2 = Unit2->GetGridMetadata();

	check(Metadata1.IsValid() && Metadata2.IsValid());

	// Chebyshev distance (max of row/col difference) - matches grid adjacency logic
	int32 RowDiff = FMath::Abs(Metadata1.Coords.Row - Metadata2.Coords.Row);
	int32 ColDiff = FMath::Abs(Metadata1.Coords.Col - Metadata2.Coords.Col);
	return FMath::Max(RowDiff, ColDiff);
}

UWeapon* UTacGridTargetingService::SelectWeapon(AUnit* Attacker, AUnit* Target) const
{
	check(Attacker && Target);

	const TArray<TObjectPtr<UWeapon>>& Weapons = Attacker->GetWeapons();
	if (Weapons.Num() == 0)
	{
		return nullptr;
	}

	int32 Distance = CalculateDistance(Attacker, Target);
	bool bIsFriendlyTarget = Attacker->GetGridMetadata().IsAlly(Target->GetGridMetadata());

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

bool UTacGridTargetingService::HasValidTargetAtCell(AUnit* Source, FTacCoordinates TargetCell, ETargetReach Reach) const
{
	check(Source);

	const FUnitGridMetadata& Metadata = Source->GetGridMetadata();
	check(Metadata.IsValid());

	switch (Reach)
	{
		case ETargetReach::None:
			return false;

		case ETargetReach::Self:
			return TargetCell == Metadata.Coords;

		case ETargetReach::ClosestEnemies:
		{
			if (!IsAdjacentCell(Metadata.Coords, TargetCell))
			{
				return false;
			}
			if (AUnit* TargetUnit = DataManager->GetUnit(TargetCell))
			{
				return Metadata.IsEnemy(TargetUnit->GetGridMetadata());
			}
			return false;
		}

		case ETargetReach::AnyEnemy:
		case ETargetReach::AllEnemies:
		case ETargetReach::Area:
		{
			if (AUnit* TargetUnit = DataManager->GetUnit(TargetCell))
			{
				return Metadata.IsEnemy(TargetUnit->GetGridMetadata());
			}
			return false;
		}

		case ETargetReach::AnyFriendly:
		case ETargetReach::AllFriendlies:
		{
			if (AUnit* TargetUnit = DataManager->GetUnit(TargetCell); TargetUnit && TargetUnit != Source)
			{
				return Metadata.IsAlly(TargetUnit->GetGridMetadata());
			}
			return false;
		}

		case ETargetReach::EmptyCell:
			return !DataManager->IsCellOccupied(TargetCell) && TargetCell.Layer == Metadata.Coords.Layer;

		case ETargetReach::EmptyCellOrFriendly:
		{
			if (!DataManager->IsCellOccupied(TargetCell) && TargetCell.Layer == Metadata.Coords.Layer)
			{
				return true;
			}
			if (AUnit* TargetUnit = DataManager->GetUnit(TargetCell); TargetUnit && TargetUnit != Source)
			{
				return Metadata.IsAlly(TargetUnit->GetGridMetadata());
			}
			return false;
		}

		case ETargetReach::Movement:
		{
			// Complex pathfinding - need full list
			const TArray<FTacCoordinates> MoveCells = GetValidTargetCells(Source, Reach);
			return MoveCells.Contains(TargetCell);
		}

		case ETargetReach::AnyCorpse:
			return DataManager->HasCorpses(TargetCell);

		case ETargetReach::FriendlyCorpse:
		{
			if (AUnit* Corpse = DataManager->GetTopCorpse(TargetCell))
			{
				return Metadata.IsAlly(Corpse->GetGridMetadata());
			}
			return false;
		}

		case ETargetReach::EnemyCorpse:
		{
			if (AUnit* Corpse = DataManager->GetTopCorpse(TargetCell))
			{
				return Metadata.IsEnemy(Corpse->GetGridMetadata());
			}
			return false;
		}

		case ETargetReach::AnyNonBlockedCorpse:
			return DataManager->HasCorpses(TargetCell) && !DataManager->IsCellOccupied(TargetCell);

		case ETargetReach::FriendlyNonBlockedCorpse:
		{
			if (DataManager->IsCellOccupied(TargetCell))
			{
				return false;
			}
			if (AUnit* Corpse = DataManager->GetTopCorpse(TargetCell))
			{
				return Metadata.IsAlly(Corpse->GetGridMetadata());
			}
			return false;
		}

		case ETargetReach::EnemyNonBlockedCorpse:
		{
			if (DataManager->IsCellOccupied(TargetCell))
			{
				return false;
			}
			if (AUnit* Corpse = DataManager->GetTopCorpse(TargetCell))
			{
				return Metadata.IsEnemy(Corpse->GetGridMetadata());
			}
			return false;
		}
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

	// Team-based checks: use quick helper
	if (Reach == ETargetReach::Area || Reach == ETargetReach::AnyEnemy || Reach == ETargetReach::AllEnemies)
	{
		if (UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(Source))
		{
			return EnemyTeam->IsAnyUnitAlive();
		}
		return false;
	}

	if (Reach == ETargetReach::AnyFriendly || Reach == ETargetReach::AllFriendlies)
	{
		if (UBattleTeam* FriendlyTeam = DataManager->GetTeamForUnit(Source))
		{
			// Need at least one alive unit besides Source
			if (!FriendlyTeam->IsAnyUnitAlive())
			{
				return false;
			}
			// If only 1 unit total, it must be Source
			if (FriendlyTeam->GetUnits().Num() == 1)
			{
				return false;
			}
			// At least 2 units and at least one alive, so there's a target
			return true;
		}
		return false;
	}

	// For everything else, check if filter returns any results
	const TArray<FTacCoordinates> ValidCells = GetValidTargetCells(Source, Reach);
	return ValidCells.Num() > 0;
}

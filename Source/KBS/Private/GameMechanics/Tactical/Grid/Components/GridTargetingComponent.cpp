#include "GameMechanics/Tactical/Grid/Components/GridTargetingComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/FlankCellDefinitions.h"
#include "GameMechanics/Units/LargeUnit.h"
UGridTargetingComponent::UGridTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UGridTargetingComponent::Initialize(UGridDataManager* InDataManager)
{
	DataManager = InDataManager;
}

TArray<FIntPoint> UGridTargetingComponent::GetValidTargetCells(AUnit* Unit) const
{
	if (!Unit || !Unit->AbilityInventory)
	{
		return TArray<FIntPoint>();
	}
	UUnitAbilityInstance* CurrentAbility = Unit->AbilityInventory->GetCurrentActiveAbility();
	if (!CurrentAbility)
	{
		return TArray<FIntPoint>();
	}
	ETargetReach Reach = CurrentAbility->GetTargeting();
	return GetValidTargetCells(Unit, Reach);
}
TArray<FIntPoint> UGridTargetingComponent::GetValidTargetCells(AUnit* Unit, ETargetReach Reach, bool bUseFlankTargeting) const
{
	TArray<FIntPoint> TargetCells;
	if (!Unit || !DataManager)
	{
		return TargetCells;
	}
	if (bUseFlankTargeting && Reach == ETargetReach::ClosestEnemies)
	{
		GetFlankTargetCells(Unit, TargetCells);
		return TargetCells;
	}
	switch (Reach)
	{
		case ETargetReach::None:
			break;
		case ETargetReach::Self:
		{
			int32 UnitRow, UnitCol;
			EBattleLayer UnitLayer;
			if (DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
			{
				TargetCells.Add(FIntPoint(UnitCol, UnitRow));
			}
			break;
		}
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
			GetAnyFriendlyCells(Unit, TargetCells);
			break;
		case ETargetReach::AllFriendlies:
			GetAllFriendlyCells(Unit, TargetCells);
			break;
		case ETargetReach::EmptyCell:
		{
			int32 UnitRow, UnitCol;
			EBattleLayer UnitLayer;
			if (DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
			{
				TargetCells = DataManager->GetValidPlacementCells(UnitLayer);
			}
			break;
		}
		case ETargetReach::EmptyCellOrFriendly:
			GetEmptyCellsOrFriendly(Unit, TargetCells);
			break;
		case ETargetReach::Movement:
		{
			GetMovementCells(Unit, TargetCells);
			break;
		}
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
void UGridTargetingComponent::GetClosestEnemyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}
	UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(Unit);
	if (!EnemyTeam)
	{
		return;
	}
	const TArray<FIntPoint> AdjacentOffsets = {
		FIntPoint(0, -1), FIntPoint(0, 1), FIntPoint(-1, 0), FIntPoint(1, 0),
		FIntPoint(-1, -1), FIntPoint(-1, 1), FIntPoint(1, -1), FIntPoint(1, 1)
	};
	for (const FIntPoint& Offset : AdjacentOffsets)
	{
		const int32 TargetRow = UnitRow + Offset.Y;
		const int32 TargetCol = UnitCol + Offset.X;
		if (!FGridCoordinates::IsValidCell(TargetRow, TargetCol))
		{
			continue;
		}
		AUnit* TargetUnit = DataManager->GetUnit(TargetRow, TargetCol, UnitLayer);
		if (TargetUnit && EnemyTeam->ContainsUnit(TargetUnit))
		{
			OutCells.Add(FIntPoint(TargetCol, TargetRow));
		}
	}
}
void UGridTargetingComponent::GetFlankTargetCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}
	UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(Unit);
	if (!EnemyTeam)
	{
		return;
	}
	TArray<FIntPoint> AllFlankTargets;
	const TArray<int32> AdjacentCols = { UnitCol - 1, UnitCol + 1 };
	for (int32 Col : AdjacentCols)
	{
		if (Col < 0 || Col >= FGridCoordinates::GridSize)
		{
			continue;
		}
		for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
		{
			AUnit* TargetUnit = DataManager->GetUnit(Row, Col, UnitLayer);
			if (TargetUnit && EnemyTeam->ContainsUnit(TargetUnit))
			{
				AllFlankTargets.Add(FIntPoint(Col, Row));
			}
		}
	}
	TArray<FIntPoint> ClosestToCenterCells;
	int32 MinDistToCenter = TNumericLimits<int32>::Max();
	for (const FIntPoint& Cell : AllFlankTargets)
	{
		const int32 Dist = FMath::Abs(Cell.Y - FGridCoordinates::CenterRow);
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
void UGridTargetingComponent::GetAnyEnemyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(Unit);
	if (!EnemyTeam)
	{
		return;
	}
	for (AUnit* EnemyUnit : EnemyTeam->GetUnits())
	{
		if (EnemyUnit)
		{
			int32 Row, Col;
			EBattleLayer Layer;
			if (DataManager->GetUnitPosition(EnemyUnit, Row, Col, Layer))
			{
				OutCells.Add(FIntPoint(Col, Row));
			}
		}
	}
}
TArray<AUnit*> UGridTargetingComponent::GetValidTargetUnits(AUnit* Unit) const
{
	if (!Unit || !Unit->AbilityInventory)
	{
		return TArray<AUnit*>();
	}
	UUnitAbilityInstance* CurrentAbility = Unit->AbilityInventory->GetCurrentActiveAbility();
	if (!CurrentAbility)
	{
		return TArray<AUnit*>();
	}
	ETargetReach Reach = CurrentAbility->GetTargeting();
	return GetValidTargetUnits(Unit, Reach);
}
TArray<AUnit*> UGridTargetingComponent::GetValidTargetUnits(AUnit* Unit, ETargetReach Reach, bool bUseFlankTargeting, bool bIncludeDeadUnits) const
{
	TArray<AUnit*> TargetUnits;
	if (!Unit || !DataManager)
	{
		return TargetUnits;
	}
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return TargetUnits;
	}
	const TArray<FIntPoint> TargetCells = GetValidTargetCells(Unit, Reach, bUseFlankTargeting);
	for (const FIntPoint& Cell : TargetCells)
	{
		AUnit* TargetUnit = DataManager->GetUnit(Cell.Y, Cell.X, UnitLayer);
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
void UGridTargetingComponent::GetAllFriendlyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	UBattleTeam* FriendlyTeam = DataManager->GetTeamForUnit(Unit);
	if (!FriendlyTeam)
	{
		return;
	}
	for (AUnit* FriendlyUnit : FriendlyTeam->GetUnits())
	{
		if (FriendlyUnit && FriendlyUnit != Unit)
		{
			int32 Row, Col;
			EBattleLayer Layer;
			if (DataManager->GetUnitPosition(FriendlyUnit, Row, Col, Layer))
			{
				OutCells.Add(FIntPoint(Col, Row));
			}
		}
	}
}
void UGridTargetingComponent::GetAnyFriendlyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	GetAllFriendlyCells(Unit, OutCells);
}
void UGridTargetingComponent::GetEmptyCellsOrFriendly(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}
	OutCells = DataManager->GetValidPlacementCells(UnitLayer);
	GetAllFriendlyCells(Unit, OutCells);
}
TArray<AUnit*> UGridTargetingComponent::ResolveTargetsFromClick(AUnit* SourceUnit, FIntPoint ClickedCell, EBattleLayer ClickedLayer, ETargetReach Reach, const FAreaShape* AreaShape) const
{
	TArray<AUnit*> ResolvedTargets;
	if (!SourceUnit || !DataManager)
	{
		return ResolvedTargets;
	}
	AUnit* ClickedUnit = DataManager->GetUnit(ClickedCell.Y, ClickedCell.X, ClickedLayer);
	AUnit* ClickedCorpse = DataManager->GetTopCorpse(ClickedCell.Y, ClickedCell.X);
	switch (Reach)
	{
		case ETargetReach::AllEnemies:
		{
			ResolvedTargets = GetValidTargetUnits(SourceUnit, Reach, false, false);
			break;
		}
		case ETargetReach::AllFriendlies:
		{
			ResolvedTargets = GetValidTargetUnits(SourceUnit, Reach, false, false);
			break;
		}
		case ETargetReach::Area:
		{
			if (AreaShape)
			{
				ResolvedTargets = GetUnitsInArea(ClickedCell, ClickedLayer, *AreaShape);
			}
			else if (ClickedUnit)
			{
				ResolvedTargets.Add(ClickedUnit);
			}
			break;
		}
		case ETargetReach::AnyCorpse:
		case ETargetReach::FriendlyCorpse:
		case ETargetReach::EnemyCorpse:
		case ETargetReach::AnyNonBlockedCorpse:
		case ETargetReach::FriendlyNonBlockedCorpse:
		case ETargetReach::EnemyNonBlockedCorpse:
		{
			if (ClickedCorpse)
			{
				ResolvedTargets.Add(ClickedCorpse);
			}
			break;
		}
		case ETargetReach::ClosestEnemies:
		case ETargetReach::AnyEnemy:
		case ETargetReach::AnyFriendly:
		case ETargetReach::EmptyCell:
		case ETargetReach::EmptyCellOrFriendly:
		default:
		{
			if (ClickedUnit)
			{
				ResolvedTargets.Add(ClickedUnit);
			}
			break;
		}
	}
	return ResolvedTargets;
}
TArray<AUnit*> UGridTargetingComponent::GetUnitsInArea(FIntPoint CenterCell, EBattleLayer Layer, const FAreaShape& AreaShape) const
{
	TArray<AUnit*> UnitsInArea;
	if (!DataManager)
	{
		return UnitsInArea;
	}

	TArray<FIntPoint> TargetCells;
	for (const FIntPoint& RelativeOffset : AreaShape.RelativeCells)
	{
		const int32 TargetCol = CenterCell.X + RelativeOffset.X;
		const int32 TargetRow = CenterCell.Y + RelativeOffset.Y;
		if (FGridCoordinates::IsValidCell(TargetRow, TargetCol))
		{
			TargetCells.Add(FIntPoint(TargetCol, TargetRow));
		}
	}

	TArray<AUnit*> UnitsAtLayer = DataManager->GetUnitsInCells(TargetCells, Layer);
	for (AUnit* Unit : UnitsAtLayer)
	{
		if (Unit && !Unit->IsDead())
		{
			UnitsInArea.Add(Unit);
		}
	}

	if (AreaShape.bAffectsAllLayers)
	{
		EBattleLayer OtherLayer = (Layer == EBattleLayer::Ground) ? EBattleLayer::Air : EBattleLayer::Ground;
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

void UGridTargetingComponent::GetAnyCorpseCells(TArray<FIntPoint>& OutCells) const
{
	if (!DataManager)
	{
		return;
	}
	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridCoordinates::GridSize; ++Col)
		{
			if (DataManager->HasCorpses(Row, Col))
			{
				OutCells.Add(FIntPoint(Col, Row));
			}
		}
	}
}
void UGridTargetingComponent::GetFriendlyCorpseCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	if (!Unit || !DataManager)
	{
		return;
	}
	UBattleTeam* FriendlyTeam = DataManager->GetTeamForUnit(Unit);
	if (!FriendlyTeam)
	{
		return;
	}
	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridCoordinates::GridSize; ++Col)
		{
			AUnit* TopCorpse = DataManager->GetTopCorpse(Row, Col);
			if (TopCorpse && FriendlyTeam->ContainsUnit(TopCorpse))
			{
				OutCells.Add(FIntPoint(Col, Row));
			}
		}
	}
}
void UGridTargetingComponent::GetEnemyCorpseCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	if (!Unit || !DataManager)
	{
		return;
	}
	UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(Unit);
	if (!EnemyTeam)
	{
		return;
	}
	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridCoordinates::GridSize; ++Col)
		{
			AUnit* TopCorpse = DataManager->GetTopCorpse(Row, Col);
			if (TopCorpse && EnemyTeam->ContainsUnit(TopCorpse))
			{
				OutCells.Add(FIntPoint(Col, Row));
			}
		}
	}
}
void UGridTargetingComponent::GetAnyNonBlockedCorpseCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	if (!Unit || !DataManager)
	{
		return;
	}
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}
	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridCoordinates::GridSize; ++Col)
		{
			if (!DataManager->HasCorpses(Row, Col))
			{
				continue;
			}
			bool bBlocked = DataManager->IsCellOccupied(Row, Col, EBattleLayer::Ground);
			if (!bBlocked)
			{
				OutCells.Add(FIntPoint(Col, Row));
			}
		}
	}
}
void UGridTargetingComponent::GetFriendlyNonBlockedCorpseCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	if (!Unit || !DataManager)
	{
		return;
	}
	UBattleTeam* FriendlyTeam = DataManager->GetTeamForUnit(Unit);
	if (!FriendlyTeam)
	{
		return;
	}
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}
	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridCoordinates::GridSize; ++Col)
		{
			AUnit* TopCorpse = DataManager->GetTopCorpse(Row, Col);
			if (!TopCorpse || !FriendlyTeam->ContainsUnit(TopCorpse))
			{
				continue;
			}
			bool bBlocked = DataManager->IsCellOccupied(Row, Col, EBattleLayer::Ground);
			if (!bBlocked)
			{
				OutCells.Add(FIntPoint(Col, Row));
			}
		}
	}
}
void UGridTargetingComponent::GetEnemyNonBlockedCorpseCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	if (!Unit || !DataManager)
	{
		return;
	}
	UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(Unit);
	if (!EnemyTeam)
	{
		return;
	}
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}
	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridCoordinates::GridSize; ++Col)
		{
			AUnit* TopCorpse = DataManager->GetTopCorpse(Row, Col);
			if (!TopCorpse || !EnemyTeam->ContainsUnit(TopCorpse))
			{
				continue;
			}
			bool bBlocked = DataManager->IsCellOccupied(Row, Col, EBattleLayer::Ground);
			if (!bBlocked)
			{
				OutCells.Add(FIntPoint(Col, Row));
			}
		}
	}
}

void UGridTargetingComponent::GetMovementCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	if (!Unit || !DataManager)
	{
		return;
	}
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}
	if (UnitLayer == EBattleLayer::Air)
	{
		GetAirMoveCells(Unit, OutCells);
	}
	else
	{
		GetAdjacentMoveCells(Unit, OutCells);
		GetFlankMoveCells(Unit, OutCells);
	}
}

void UGridTargetingComponent::GetAdjacentMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}
	UBattleTeam* UnitTeam = DataManager->GetTeamForUnit(Unit);
	const TArray<FIntPoint> AdjacentOffsets = {
		FIntPoint(0, -1), FIntPoint(0, 1), FIntPoint(-1, 0), FIntPoint(1, 0)
	};
	for (const FIntPoint& Offset : AdjacentOffsets)
	{
		const int32 TargetRow = UnitRow + Offset.Y;
		const int32 TargetCol = UnitCol + Offset.X;
		if (!DataManager->IsValidCell(TargetRow, TargetCol, UnitLayer))
		{
			continue;
		}
		if (DataManager->IsFlankCell(TargetRow, TargetCol))
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
		AUnit* OccupyingUnit = DataManager->GetUnit(TargetRow, TargetCol, UnitLayer);
		if (OccupyingUnit && (!UnitTeam || !UnitTeam->ContainsUnit(OccupyingUnit)))
		{
			continue;
		}

		if (Unit->IsMultiCell())
		{
			bool bTargetIsFlank = DataManager->IsFlankCell(TargetRow, TargetCol);
			FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(TargetRow, TargetCol, bTargetIsFlank, Unit->GetTeamSide());

			if (!DataManager->IsValidCell(SecondaryCell.Y, SecondaryCell.X, UnitLayer))
			{
				continue;
			}
			AUnit* SecondaryOccupant = DataManager->GetUnit(SecondaryCell.Y, SecondaryCell.X, UnitLayer);
			if (SecondaryOccupant && SecondaryOccupant != Unit)
			{
				continue;
			}
		}

		OutCells.Add(FIntPoint(TargetCol, TargetRow));
	}
}

void UGridTargetingComponent::GetFlankMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}
	UBattleTeam* UnitTeam = DataManager->GetTeamForUnit(Unit);
	if (!UnitTeam)
	{
		return;
	}
	const FIntPoint UnitPos(UnitCol, UnitRow);
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
	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col : EnemyFlankColumns)
		{
			if (!DataManager->IsValidCell(Row, Col, UnitLayer))
			{
				continue;
			}
			if (!DataManager->IsFlankCell(Row, Col))
			{
				continue;
			}
			const FIntPoint FlankCell(Col, Row);
			if (CanEnterFlankCell(UnitPos, FlankCell, UnitTeam))
			{
				AUnit* Occupant = DataManager->GetUnit(Row, Col, UnitLayer);
				if (Occupant && (!UnitTeam || !UnitTeam->ContainsUnit(Occupant)))
				{
					continue;
				}

				if (Unit->IsMultiCell())
				{
					FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(Row, Col, true, Unit->GetTeamSide());
					if (!DataManager->IsValidCell(SecondaryCell.Y, SecondaryCell.X, UnitLayer))
					{
						continue;
					}
					AUnit* SecondaryOccupant = DataManager->GetUnit(SecondaryCell.Y, SecondaryCell.X, UnitLayer);
					if (SecondaryOccupant && SecondaryOccupant != Unit)
					{
						continue;
					}
				}

				OutCells.Add(FlankCell);
			}
		}
	}
}

void UGridTargetingComponent::GetAirMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}
	UBattleTeam* UnitTeam = DataManager->GetTeamForUnit(Unit);
	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridCoordinates::GridSize; ++Col)
		{
			if (DataManager->IsRestrictedCell(Row, Col))
			{
				continue;
			}
			AUnit* OccupyingUnit = DataManager->GetUnit(Row, Col, UnitLayer);
			if (OccupyingUnit && (!UnitTeam || !UnitTeam->ContainsUnit(OccupyingUnit)))
			{
				continue;
			}

			if (Unit->IsMultiCell())
			{
				bool bTargetIsFlank = DataManager->IsFlankCell(Row, Col);
				FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(Row, Col, bTargetIsFlank, Unit->GetTeamSide());

				if (!DataManager->IsValidCell(SecondaryCell.Y, SecondaryCell.X, UnitLayer))
				{
					continue;
				}
				AUnit* SecondaryOccupant = DataManager->GetUnit(SecondaryCell.Y, SecondaryCell.X, UnitLayer);
				if (SecondaryOccupant && SecondaryOccupant != Unit)
				{
					continue;
				}
			}

			OutCells.Add(FIntPoint(Col, Row));
		}
	}
}

bool UGridTargetingComponent::CanEnterFlankCell(const FIntPoint& UnitPos, const FIntPoint& FlankCell, UBattleTeam* UnitTeam) const
{
	if (!DataManager || !UnitTeam)
	{
		return false;
	}
	const int32 UnitRow = UnitPos.Y;
	const int32 UnitCol = UnitPos.X;
	const int32 FlankRow = FlankCell.Y;
	const int32 FlankCol = FlankCell.X;
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

bool UGridTargetingComponent::IsAdjacentCell(const FIntPoint& CellA, const FIntPoint& CellB) const
{
	const int32 ManhattanDistance = FMath::Abs(CellA.X - CellB.X) + FMath::Abs(CellA.Y - CellB.Y);
	return ManhattanDistance == 1;
}

bool UGridTargetingComponent::IsFlankCell(const FIntPoint& Cell) const
{
	if (!DataManager)
	{
		return false;
	}
	return DataManager->IsFlankCell(Cell.Y, Cell.X);
}

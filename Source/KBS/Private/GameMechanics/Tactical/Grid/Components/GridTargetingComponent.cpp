#include "GameMechanics/Tactical/Grid/Components/GridTargetingComponent.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameplayTypes/GridCoordinates.h"

UGridTargetingComponent::UGridTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGridTargetingComponent::Initialize(ATacBattleGrid* InGrid, UGridDataManager* InDataManager)
{
	Grid = InGrid;
	DataManager = InDataManager;
}

TArray<FIntPoint> UGridTargetingComponent::GetValidTargetCells(AUnit* Unit, ETargetReach Reach) const
{
	TArray<FIntPoint> TargetCells;

	if (!Unit || !Grid || !DataManager)
	{
		return TargetCells;
	}

	if (Reach == ETargetReach::ClosestEnemies)
	{
		GetClosestEnemyCells(Unit, TargetCells);
	}
	else if (Reach == ETargetReach::AnyEnemy)
	{
		GetAnyEnemyCells(Unit, TargetCells);
	}

	return TargetCells;
}

void UGridTargetingComponent::GetClosestEnemyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!Grid->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}

	UBattleTeam* EnemyTeam = Grid->GetEnemyTeam(Unit);

	if (!EnemyTeam)
	{
		return;
	}

	// ClosestEnemies = Adjacent enemies (orthogonal + diagonal)
	const TArray<FIntPoint> AdjacentOffsets = {
		// Orthogonal (up, down, left, right)
		FIntPoint(0, -1), FIntPoint(0, 1), FIntPoint(-1, 0), FIntPoint(1, 0),
		// Diagonal
		FIntPoint(-1, -1), FIntPoint(-1, 1), FIntPoint(1, -1), FIntPoint(1, 1)
	};

	for (const FIntPoint& Offset : AdjacentOffsets)
	{
		const int32 TargetRow = UnitRow + Offset.Y;
		const int32 TargetCol = UnitCol + Offset.X;

		if (!Grid->IsValidCell(TargetRow, TargetCol, UnitLayer))
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
	if (!Grid->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}

	UBattleTeam* EnemyTeam = Grid->GetEnemyTeam(Unit);

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

	// Filter to only closest to center
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
	UBattleTeam* EnemyTeam = Grid->GetEnemyTeam(Unit);

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
			if (Grid->GetUnitPosition(EnemyUnit, Row, Col, Layer))
			{
				OutCells.Add(FIntPoint(Col, Row));
			}
		}
	}
}

TArray<AUnit*> UGridTargetingComponent::GetValidTargetUnits(AUnit* Unit, ETargetReach Reach) const
{
	TArray<AUnit*> TargetUnits;

	if (!Unit || !Grid || !DataManager)
	{
		return TargetUnits;
	}

	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!Grid->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return TargetUnits;
	}

	const TArray<FIntPoint> TargetCells = GetValidTargetCells(Unit, Reach);

	for (const FIntPoint& Cell : TargetCells)
	{
		AUnit* TargetUnit = DataManager->GetUnit(Cell.Y, Cell.X, UnitLayer);

		if (TargetUnit)
		{
			TargetUnits.Add(TargetUnit);
		}
	}

	return TargetUnits;
}

#include "GameMechanics/Tactical/Grid/Components/GridTargetingComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/DamageTypes.h"

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

	// Flank targeting override (only for ClosestEnemies)
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
				TargetCells.Add(FIntPoint(UnitRow, UnitCol));
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
			// Filter dead units unless explicitly included
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

	// Get empty cells
	OutCells = DataManager->GetValidPlacementCells(UnitLayer);

	// Add friendly unit cells
	GetAllFriendlyCells(Unit, OutCells);
}

TArray<AUnit*> UGridTargetingComponent::ResolveTargetsFromClick(AUnit* SourceUnit, FIntPoint ClickedCell, EBattleLayer ClickedLayer, ETargetReach Reach, const FAreaShape* AreaShape) const
{
	TArray<AUnit*> ResolvedTargets;

	if (!SourceUnit || !DataManager)
	{
		return ResolvedTargets;
	}

	// Get the clicked unit (if any)
	AUnit* ClickedUnit = DataManager->GetUnit(ClickedCell.Y, ClickedCell.X, ClickedLayer);

	switch (Reach)
	{
		case ETargetReach::AllEnemies:
		{
			// Click any enemy, hit all enemies
			ResolvedTargets = GetValidTargetUnits(SourceUnit, Reach, false, false);
			break;
		}

		case ETargetReach::AllFriendlies:
		{
			// Click any friendly, hit all friendlies
			ResolvedTargets = GetValidTargetUnits(SourceUnit, Reach, false, false);
			break;
		}

		case ETargetReach::Area:
		{
			// Click a unit, apply area shape centered on that unit's cell
			if (AreaShape)
			{
				ResolvedTargets = GetUnitsInArea(ClickedCell, ClickedLayer, *AreaShape);
			}
			else if (ClickedUnit)
			{
				// No area shape provided, just return clicked unit
				ResolvedTargets.Add(ClickedUnit);
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
			// Single target - just return clicked unit
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

	// Apply each relative cell offset to find affected cells
	for (const FIntPoint& RelativeOffset : AreaShape.RelativeCells)
	{
		const int32 TargetCol = CenterCell.X + RelativeOffset.X;
		const int32 TargetRow = CenterCell.Y + RelativeOffset.Y;

		// Check if target cell is valid
		if (!FGridCoordinates::IsValidCell(TargetRow, TargetCol))
		{
			continue;
		}

		// Get unit at this cell
		AUnit* UnitAtCell = DataManager->GetUnit(TargetRow, TargetCol, Layer);
		if (UnitAtCell && !UnitAtCell->IsDead())
		{
			UnitsInArea.Add(UnitAtCell);
		}

		// If area affects all layers, check other layer too
		if (AreaShape.bAffectsAllLayers)
		{
			EBattleLayer OtherLayer = (Layer == EBattleLayer::Ground) ? EBattleLayer::Air : EBattleLayer::Ground;

			if (FGridCoordinates::IsValidCell(TargetRow, TargetCol))
			{
				AUnit* UnitAtOtherLayer = DataManager->GetUnit(TargetRow, TargetCol, OtherLayer);
				if (UnitAtOtherLayer && !UnitAtOtherLayer->IsDead())
				{
					UnitsInArea.Add(UnitAtOtherLayer);
				}
			}
		}
	}

	return UnitsInArea;
}

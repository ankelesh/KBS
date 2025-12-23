#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
void UGridDataManager::Initialize(ATacBattleGrid* InGrid)
{
	Grid = InGrid;
	if (Grid)
	{
		GridWorldLocation = Grid->GetActorLocation();
		AttackerTeam = NewObject<UBattleTeam>(Grid);
		AttackerTeam->SetTeamSide(ETeamSide::Attacker);
		DefenderTeam = NewObject<UBattleTeam>(Grid);
		DefenderTeam->SetTeamSide(ETeamSide::Defender);
	}
	GroundLayer.SetNum(FGridCoordinates::GridSize);
	AirLayer.SetNum(FGridCoordinates::GridSize);
}
bool UGridDataManager::PlaceUnit(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer, ATacBattleGrid* TacBattleGrid)
{
	if (!Unit || !FGridCoordinates::IsValidCell(Row, Col))
	{
		return false;
	}
	TArray<FGridRow>& LayerArray = GetLayer(Layer);
	if (Row >= LayerArray.Num() || Col >= LayerArray[Row].Cells.Num())
	{
		return false;
	}
	if (LayerArray[Row].Cells[Col] != nullptr)
	{
		return false;
	}
	LayerArray[Row].Cells[Col] = Unit;
	Unit->SetActorLocation(FGridCoordinates::CellToWorldLocation(Row, Col, Layer, GridWorldLocation));
	return true;
}
AUnit* UGridDataManager::GetUnit(int32 Row, int32 Col, EBattleLayer Layer) const
{
	if (!FGridCoordinates::IsValidCell(Row, Col))
	{
		return nullptr;
	}
	const TArray<FGridRow>& LayerArray = GetLayer(Layer);
	if (Row >= LayerArray.Num() || Col >= LayerArray[Row].Cells.Num())
	{
		return nullptr;
	}
	return LayerArray[Row].Cells[Col];
}
bool UGridDataManager::RemoveUnit(int32 Row, int32 Col, EBattleLayer Layer, ATacBattleGrid* TacBattleGrid)
{
	if (!FGridCoordinates::IsValidCell(Row, Col))
	{
		return false;
	}
	TArray<FGridRow>& LayerArray = GetLayer(Layer);
	if (Row >= LayerArray.Num() || Col >= LayerArray[Row].Cells.Num())
	{
		return false;
	}
	AUnit* Unit = LayerArray[Row].Cells[Col];
	if (Unit == nullptr)
	{
		return false;
	}
	UnitFlankStates.Remove(Unit);
	UnitOriginalRotations.Remove(Unit);
	LayerArray[Row].Cells[Col] = nullptr;
	return true;
}
TArray<FGridRow>& UGridDataManager::GetLayer(EBattleLayer Layer)
{
	return Layer == EBattleLayer::Ground ? GroundLayer : AirLayer;
}
const TArray<FGridRow>& UGridDataManager::GetLayer(EBattleLayer Layer) const
{
	return Layer == EBattleLayer::Ground ? GroundLayer : AirLayer;
}
bool UGridDataManager::GetUnitPosition(const AUnit* Unit, int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const
{
	if (!Unit)
	{
		return false;
	}
	for (int32 Row = 0; Row < GroundLayer.Num(); ++Row)
	{
		for (int32 Col = 0; Col < GroundLayer[Row].Cells.Num(); ++Col)
		{
			if (GroundLayer[Row].Cells[Col] == Unit)
			{
				OutRow = Row;
				OutCol = Col;
				OutLayer = EBattleLayer::Ground;
				return true;
			}
		}
	}
	for (int32 Row = 0; Row < AirLayer.Num(); ++Row)
	{
		for (int32 Col = 0; Col < AirLayer[Row].Cells.Num(); ++Col)
		{
			if (AirLayer[Row].Cells[Col] == Unit)
			{
				OutRow = Row;
				OutCol = Col;
				OutLayer = EBattleLayer::Air;
				return true;
			}
		}
	}
	return false;
}
bool UGridDataManager::IsUnitOnFlank(const AUnit* Unit) const
{
	const bool* bOnFlank = UnitFlankStates.Find(const_cast<AUnit*>(Unit));
	return bOnFlank ? *bOnFlank : false;
}
void UGridDataManager::SetUnitFlankState(AUnit* Unit, bool bOnFlank)
{
	if (bOnFlank)
	{
		UnitFlankStates.Add(Unit, true);
	}
	else
	{
		UnitFlankStates.Remove(Unit);
	}
}
FRotator UGridDataManager::GetUnitOriginalRotation(const AUnit* Unit) const
{
	const FRotator* Rotation = UnitOriginalRotations.Find(const_cast<AUnit*>(Unit));
	return Rotation ? *Rotation : FRotator::ZeroRotator;
}
void UGridDataManager::SetUnitOriginalRotation(AUnit* Unit, const FRotator& Rotation)
{
	UnitOriginalRotations.Add(Unit, Rotation);
}
bool UGridDataManager::IsValidCell(int32 Row, int32 Col, EBattleLayer Layer) const
{
	return FGridCoordinates::IsValidCell(Row, Col);
}
bool UGridDataManager::IsFlankCell(int32 Row, int32 Col) const
{
	return FGridCoordinates::IsFlankCell(Row, Col);
}
bool UGridDataManager::IsRestrictedCell(int32 Row, int32 Col) const
{
	return FGridCoordinates::IsRestrictedCell(Row, Col);
}
FVector UGridDataManager::GetCellWorldLocation(int32 Row, int32 Col, EBattleLayer Layer) const
{
	return FGridCoordinates::CellToWorldLocation(Row, Col, Layer, GridWorldLocation);
}
UBattleTeam* UGridDataManager::GetTeamForUnit(AUnit* Unit) const
{
	if (!Unit)
	{
		return nullptr;
	}
	if (AttackerTeam && AttackerTeam->ContainsUnit(Unit))
	{
		return AttackerTeam;
	}
	if (DefenderTeam && DefenderTeam->ContainsUnit(Unit))
	{
		return DefenderTeam;
	}
	return nullptr;
}
UBattleTeam* UGridDataManager::GetEnemyTeam(AUnit* Unit) const
{
	UBattleTeam* UnitTeam = GetTeamForUnit(Unit);
	if (UnitTeam == AttackerTeam)
	{
		return DefenderTeam;
	}
	if (UnitTeam == DefenderTeam)
	{
		return AttackerTeam;
	}
	return nullptr;
}
TArray<AUnit*> UGridDataManager::GetUnitsFromTeam(bool bIsAttacker) const
{
	UBattleTeam* Team = bIsAttacker ? AttackerTeam : DefenderTeam;
	if (!Team)
	{
		return TArray<AUnit*>();
	}
	return Team->GetUnits();
}
TArray<FIntPoint> UGridDataManager::GetEmptyCells(EBattleLayer Layer) const
{
	TArray<FIntPoint> EmptyCells;
	const TArray<FGridRow>& LayerArray = GetLayer(Layer);
	for (int32 Row = 0; Row < LayerArray.Num(); ++Row)
	{
		for (int32 Col = 0; Col < LayerArray[Row].Cells.Num(); ++Col)
		{
			if (IsValidCell(Row, Col, Layer) && LayerArray[Row].Cells[Col] == nullptr)
			{
				EmptyCells.Add(FIntPoint(Col, Row));
			}
		}
	}
	return EmptyCells;
}
TArray<FIntPoint> UGridDataManager::GetOccupiedCells(EBattleLayer Layer, UBattleTeam* Team) const
{
	TArray<FIntPoint> OccupiedCells;
	if (!Team)
	{
		return OccupiedCells;
	}
	const TArray<FGridRow>& LayerArray = GetLayer(Layer);
	for (int32 Row = 0; Row < LayerArray.Num(); ++Row)
	{
		for (int32 Col = 0; Col < LayerArray[Row].Cells.Num(); ++Col)
		{
			AUnit* Unit = LayerArray[Row].Cells[Col];
			if (Unit && Team->ContainsUnit(Unit))
			{
				OccupiedCells.Add(FIntPoint(Col, Row));
			}
		}
	}
	return OccupiedCells;
}
bool UGridDataManager::IsCellOccupied(int32 Row, int32 Col, EBattleLayer Layer) const
{
	return GetUnit(Row, Col, Layer) != nullptr;
}
TArray<FIntPoint> UGridDataManager::GetValidPlacementCells(EBattleLayer Layer) const
{
	TArray<FIntPoint> PlacementCells = GetEmptyCells(Layer);
	PlacementCells.RemoveAll([this](const FIntPoint& Cell)
	{
		return IsRestrictedCell(Cell.Y, Cell.X);
	});
	return PlacementCells;
}

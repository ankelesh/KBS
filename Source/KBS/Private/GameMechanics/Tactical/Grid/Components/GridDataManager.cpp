#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/LargeUnit.h"

void FCorpseStack::Push(AUnit* Unit, const FVector& WorldLocation)
{
	if (!Unit)
	{
		return;
	}
	if (Corpses.Num() > 0)
	{
		AUnit* PreviousTop = Corpses.Last();
		SetCorpseVisibility(PreviousTop, false);
	}
	Corpses.Add(Unit);
	Unit->SetActorLocation(WorldLocation);
	SetCorpseVisibility(Unit, true);
}

AUnit* FCorpseStack::Pop()
{
	if (Corpses.IsEmpty())
	{
		return nullptr;
	}
	AUnit* TopCorpse = Corpses.Last();
	Corpses.RemoveAt(Corpses.Num() - 1);
	if (Corpses.Num() > 0)
	{
		AUnit* NewTop = Corpses.Last();
		SetCorpseVisibility(NewTop, true);
	}
	return TopCorpse;
}

AUnit* FCorpseStack::Top() const
{
	return Corpses.Num() > 0 ? Corpses.Last() : nullptr;
}

void FCorpseStack::SetCorpseVisibility(AUnit* Corpse, bool bVisible)
{
	if (!Corpse)
	{
		return;
	}
	if (UUnitVisualsComponent* VisualsComp = Corpse->GetVisualsComponent())
	{
		for (USceneComponent* MeshComp : VisualsComp->GetAllMeshComponents())
		{
			if (MeshComp)
			{
				MeshComp->SetVisibility(bVisible, true);
			}
		}
	}
}

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

	if (Unit->IsMultiCell())
	{
		bool bIsHorizontal = IsFlankCell(Row, Col);
		FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(Row, Col, bIsHorizontal, Unit->GetTeamSide());

		if (!FGridCoordinates::IsValidCell(SecondaryCell.Y, SecondaryCell.X))
		{
			return false;
		}
		if (SecondaryCell.Y >= LayerArray.Num() || SecondaryCell.X >= LayerArray[SecondaryCell.Y].Cells.Num())
		{
			return false;
		}
		if (LayerArray[SecondaryCell.Y].Cells[SecondaryCell.X] != nullptr)
		{
			return false;
		}

		LayerArray[Row].Cells[Col] = Unit;
		LayerArray[SecondaryCell.Y].Cells[SecondaryCell.X] = Unit;

		FVector PrimaryLoc = FGridCoordinates::CellToWorldLocation(Row, Col, Layer, GridWorldLocation);
		FVector SecondaryLoc = FGridCoordinates::CellToWorldLocation(SecondaryCell.Y, SecondaryCell.X, Layer, GridWorldLocation);
		FVector CenteredLoc = (PrimaryLoc + SecondaryLoc) * 0.5f;
		Unit->SetActorLocation(CenteredLoc);

		FMultiCellUnitData MultiCellData;
		MultiCellData.PrimaryCell = FGridCellCoord(Row, Col, Layer);
		MultiCellData.OccupiedCells.Add(FGridCellCoord(Row, Col, Layer));
		MultiCellData.OccupiedCells.Add(FGridCellCoord(SecondaryCell.Y, SecondaryCell.X, Layer));
		MultiCellData.bIsHorizontal = bIsHorizontal;
		MultiCellUnits.Add(Unit, MultiCellData);
	}
	else
	{
		LayerArray[Row].Cells[Col] = Unit;
		Unit->SetActorLocation(FGridCoordinates::CellToWorldLocation(Row, Col, Layer, GridWorldLocation));
	}

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

	if (Unit->IsMultiCell())
	{
		const FMultiCellUnitData* MultiCellData = MultiCellUnits.Find(Unit);
		if (MultiCellData)
		{
			for (const FGridCellCoord& CellCoord : MultiCellData->OccupiedCells)
			{
				TArray<FGridRow>& CellLayer = GetLayer(CellCoord.Layer);
				if (CellCoord.Row < CellLayer.Num() && CellCoord.Col < CellLayer[CellCoord.Row].Cells.Num())
				{
					CellLayer[CellCoord.Row].Cells[CellCoord.Col] = nullptr;
				}
			}
		}
		MultiCellUnits.Remove(Unit);
	}
	else
	{
		LayerArray[Row].Cells[Col] = nullptr;
	}

	UnitFlankStates.Remove(Unit);
	UnitOriginalRotations.Remove(Unit);
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

	if (Unit->IsMultiCell())
	{
		const FMultiCellUnitData* MultiCellData = MultiCellUnits.Find(const_cast<AUnit*>(Unit));
		if (MultiCellData)
		{
			OutRow = MultiCellData->PrimaryCell.Row;
			OutCol = MultiCellData->PrimaryCell.Col;
			OutLayer = MultiCellData->PrimaryCell.Layer;
			return true;
		}
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
void UGridDataManager::PushCorpse(AUnit* Unit, int32 Row, int32 Col)
{
	if (!Unit || !FGridCoordinates::IsValidCell(Row, Col))
	{
		return;
	}
	if (Row >= GroundLayer.Num() || Col >= GroundLayer[Row].CorpseStacks.Num())
	{
		return;
	}
	FVector WorldLocation = FGridCoordinates::CellToWorldLocation(Row, Col, EBattleLayer::Ground, GridWorldLocation);
	GroundLayer[Row].CorpseStacks[Col].Push(Unit, WorldLocation);
}
AUnit* UGridDataManager::GetTopCorpse(int32 Row, int32 Col) const
{
	if (!FGridCoordinates::IsValidCell(Row, Col))
	{
		return nullptr;
	}
	if (Row >= GroundLayer.Num() || Col >= GroundLayer[Row].CorpseStacks.Num())
	{
		return nullptr;
	}
	return GroundLayer[Row].CorpseStacks[Col].Top();
}
AUnit* UGridDataManager::PopCorpse(int32 Row, int32 Col)
{
	if (!FGridCoordinates::IsValidCell(Row, Col))
	{
		return nullptr;
	}
	if (Row >= GroundLayer.Num() || Col >= GroundLayer[Row].CorpseStacks.Num())
	{
		return nullptr;
	}
	return GroundLayer[Row].CorpseStacks[Col].Pop();
}
bool UGridDataManager::HasCorpses(int32 Row, int32 Col) const
{
	if (!FGridCoordinates::IsValidCell(Row, Col))
	{
		return false;
	}
	if (Row >= GroundLayer.Num() || Col >= GroundLayer[Row].CorpseStacks.Num())
	{
		return false;
	}
	return !GroundLayer[Row].CorpseStacks[Col].IsEmpty();
}
const TArray<TObjectPtr<AUnit>>& UGridDataManager::GetCorpseStack(int32 Row, int32 Col) const
{
	static const TArray<TObjectPtr<AUnit>> EmptyStack;
	if (!FGridCoordinates::IsValidCell(Row, Col))
	{
		return EmptyStack;
	}
	if (Row >= GroundLayer.Num() || Col >= GroundLayer[Row].CorpseStacks.Num())
	{
		return EmptyStack;
	}
	return GroundLayer[Row].CorpseStacks[Col].GetAll();
}

TArray<AUnit*> UGridDataManager::GetUnitsInCells(const TArray<FIntPoint>& CellCoords, EBattleLayer Layer) const
{
	TSet<AUnit*> UniqueUnits;
	const TArray<FGridRow>& LayerArray = GetLayer(Layer);

	for (const FIntPoint& Cell : CellCoords)
	{
		int32 Row = Cell.Y;
		int32 Col = Cell.X;

		if (FGridCoordinates::IsValidCell(Row, Col) && Row < LayerArray.Num() && Col < LayerArray[Row].Cells.Num())
		{
			AUnit* Unit = LayerArray[Row].Cells[Col];
			if (Unit)
			{
				UniqueUnits.Add(Unit);
			}
		}
	}

	return UniqueUnits.Array();
}

bool UGridDataManager::IsMultiCellUnit(const AUnit* Unit) const
{
	if (!Unit)
	{
		return false;
	}
	return MultiCellUnits.Contains(const_cast<AUnit*>(Unit));
}

const FMultiCellUnitData* UGridDataManager::GetMultiCellData(const AUnit* Unit) const
{
	if (!Unit)
	{
		return nullptr;
	}
	return MultiCellUnits.Find(const_cast<AUnit*>(Unit));
}

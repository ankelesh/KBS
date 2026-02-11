#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/LargeUnit.h"
#include "GameMechanics/Units/Unit.h"

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
	GroundLayer.SetNum(FGridConstants::GridSize);
	AirLayer.SetNum(FGridConstants::GridSize);
}

// Primary FTacCoordinates-based implementation
bool UGridDataManager::PlaceUnit(AUnit* Unit, FTacCoordinates Coords)
{
	if (!Unit || !Coords.IsValidCell())
	{
		return false;
	}
	TArray<FGridRow>& LayerArray = GetLayer(Coords.Layer);
	if (Coords.Row >= LayerArray.Num() || Coords.Col >= LayerArray[Coords.Row].Cells.Num())
	{
		return false;
	}

	if (LayerArray[Coords.Row].Cells[Coords.Col] != nullptr)
	{
		return false;
	}

	if (Unit->IsMultiCell())
	{
		bool bIsHorizontal = Coords.IsFlankCell();
		FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(Coords.Row, Coords.Col, bIsHorizontal, Unit->GetTeamSide());

		if (!FTacCoordinates::IsValidCell(SecondaryCell.Y, SecondaryCell.X))
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

		LayerArray[Coords.Row].Cells[Coords.Col] = Unit;
		LayerArray[SecondaryCell.Y].Cells[SecondaryCell.X] = Unit;

		FVector PrimaryLoc = Coords.ToWorldLocation(GridWorldLocation, Grid->GetCellSize(), Grid->GetAirLayerHeight());
		FVector SecondaryLoc = FTacCoordinates::CellToWorldLocation(SecondaryCell.Y, SecondaryCell.X, Coords.Layer, GridWorldLocation, Grid->GetCellSize(), Grid->GetAirLayerHeight());
		FVector CenteredLoc = (PrimaryLoc + SecondaryLoc) * 0.5f;
		Unit->SetActorLocation(CenteredLoc);

		FMultiCellUnitData MultiCellData;
		MultiCellData.PrimaryCell = Coords;
		MultiCellData.OccupiedCells.Add(FTacCoordinates(Coords.Row, Coords.Col, Coords.Layer));
		MultiCellData.OccupiedCells.Add(FTacCoordinates(SecondaryCell.Y, SecondaryCell.X, Coords.Layer));
		MultiCellData.bIsHorizontal = bIsHorizontal;
		MultiCellUnits.Add(Unit->GetGUID(), MultiCellData);
	}
	else
	{
		LayerArray[Coords.Row].Cells[Coords.Col] = Unit;
		Unit->SetActorLocation(Coords.ToWorldLocation(GridWorldLocation, Grid->GetCellSize(), Grid->GetAirLayerHeight()));
	}

	return true;
}

// Convenience overload (delegates to primary)
bool UGridDataManager::PlaceUnit(AUnit* Unit, int32 Row, int32 Col, ETacGridLayer Layer, ATacBattleGrid* TacBattleGrid)
{
	return PlaceUnit(Unit, FTacCoordinates(Row, Col, Layer));
}
// Primary FTacCoordinates-based implementation
AUnit* UGridDataManager::GetUnit(FTacCoordinates Coords) const
{
	if (!Coords.IsValidCell())
	{
		return nullptr;
	}
	const TArray<FGridRow>& LayerArray = GetLayer(Coords.Layer);
	if (Coords.Row >= LayerArray.Num() || Coords.Col >= LayerArray[Coords.Row].Cells.Num())
	{
		return nullptr;
	}
	return LayerArray[Coords.Row].Cells[Coords.Col];
}

// Convenience overload (delegates to primary)
AUnit* UGridDataManager::GetUnit(int32 Row, int32 Col, ETacGridLayer Layer) const
{
	return GetUnit(FTacCoordinates(Row, Col, Layer));
}
// Primary FTacCoordinates-based implementation
bool UGridDataManager::RemoveUnit(FTacCoordinates Coords)
{
	if (!Coords.IsValidCell())
	{
		return false;
	}
	TArray<FGridRow>& LayerArray = GetLayer(Coords.Layer);
	if (Coords.Row >= LayerArray.Num() || Coords.Col >= LayerArray[Coords.Row].Cells.Num())
	{
		return false;
	}
	AUnit* Unit = LayerArray[Coords.Row].Cells[Coords.Col];
	if (Unit == nullptr)
	{
		return false;
	}

	if (Unit->IsMultiCell())
	{
		const FMultiCellUnitData* MultiCellData = MultiCellUnits.Find(Unit->GetGUID());
		if (MultiCellData)
		{
			for (const FTacCoordinates& CellCoord : MultiCellData->OccupiedCells)
			{
				TArray<FGridRow>& CellLayer = GetLayer(CellCoord.Layer);
				if (CellCoord.Row < CellLayer.Num() && CellCoord.Col < CellLayer[CellCoord.Row].Cells.Num())
				{
					CellLayer[CellCoord.Row].Cells[CellCoord.Col] = nullptr;
				}
			}
		}
		MultiCellUnits.Remove(Unit->GetGUID());
	}
	else
	{
		LayerArray[Coords.Row].Cells[Coords.Col] = nullptr;
	}

	UnitFlankStates.Remove(Unit->GetGUID());
	UnitOriginalRotations.Remove(Unit->GetGUID());
	return true;
}

// Convenience overload (delegates to primary)
bool UGridDataManager::RemoveUnit(int32 Row, int32 Col, ETacGridLayer Layer, ATacBattleGrid* TacBattleGrid)
{
	return RemoveUnit(FTacCoordinates(Row, Col, Layer));
}
TArray<FGridRow>& UGridDataManager::GetLayer(ETacGridLayer Layer)
{
	return Layer == ETacGridLayer::Ground ? GroundLayer : AirLayer;
}
const TArray<FGridRow>& UGridDataManager::GetLayer(ETacGridLayer Layer) const
{
	return Layer == ETacGridLayer::Ground ? GroundLayer : AirLayer;
}
bool UGridDataManager::GetUnitPosition(const AUnit* Unit, FTacCoordinates& OutPosition, ETacGridLayer& OutLayer) const
{
	if (!Unit)
	{
		return false;
	}

	if (Unit->IsMultiCell())
	{
		const FMultiCellUnitData* MultiCellData = MultiCellUnits.Find(Unit->GetGUID());
		if (MultiCellData)
		{
			OutPosition = MultiCellData->PrimaryCell;
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
				OutPosition = FTacCoordinates(Row, Col, ETacGridLayer::Ground);
				OutLayer = ETacGridLayer::Ground;
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
				OutPosition = FTacCoordinates(Row, Col, ETacGridLayer::Air);
				OutLayer = ETacGridLayer::Air;
				return true;
			}
		}
	}
	return false;
}
bool UGridDataManager::IsUnitOnFlank(const AUnit* Unit) const
{
	if (!Unit)
	{
		return false;
	}
	const bool* bOnFlank = UnitFlankStates.Find(Unit->GetGUID());
	return bOnFlank ? *bOnFlank : false;
}
void UGridDataManager::SetUnitFlankState(AUnit* Unit, bool bOnFlank)
{
	if (!Unit)
	{
		return;
	}
	if (bOnFlank)
	{
		UnitFlankStates.Add(Unit->GetGUID(), true);
	}
	else
	{
		UnitFlankStates.Remove(Unit->GetGUID());
	}
}
FRotator UGridDataManager::GetUnitOriginalRotation(const AUnit* Unit) const
{
	if (!Unit)
	{
		return FRotator::ZeroRotator;
	}
	const FRotator* Rotation = UnitOriginalRotations.Find(Unit->GetGUID());
	return Rotation ? *Rotation : FRotator::ZeroRotator;
}
void UGridDataManager::SetUnitOriginalRotation(AUnit* Unit, const FRotator& Rotation)
{
	if (!Unit)
	{
		return;
	}
	UnitOriginalRotations.Add(Unit->GetGUID(), Rotation);
}
bool UGridDataManager::IsValidCell(FTacCoordinates Coords) const
{
	return Coords.IsValidCell();
}
bool UGridDataManager::IsFlankCell(FTacCoordinates Coords) const
{
	return Coords.IsFlankCell();
}
bool UGridDataManager::IsRestrictedCell(FTacCoordinates Coords) const
{
	return Coords.IsRestrictedCell();
}
FVector UGridDataManager::GetCellWorldLocation(FTacCoordinates Coords) const
{
	if (!Grid || !Grid->Config)
	{
		return FVector::ZeroVector;
	}
	return FTacCoordinates::CellToWorldLocation(Coords.Row, Coords.Col, Coords.Layer, GridWorldLocation, Grid->Config->CellSize, Grid->Config->AirLayerHeight);
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
TArray<FTacCoordinates> UGridDataManager::GetEmptyCells(ETacGridLayer Layer) const
{
	TArray<FTacCoordinates> EmptyCells;
	const TArray<FGridRow>& LayerArray = GetLayer(Layer);
	for (int32 Row = 0; Row < LayerArray.Num(); ++Row)
	{
		for (int32 Col = 0; Col < LayerArray[Row].Cells.Num(); ++Col)
		{
			FTacCoordinates Coords(Row, Col, Layer);
			if (Coords.IsValidCell() && LayerArray[Row].Cells[Col] == nullptr)
			{
				EmptyCells.Add(Coords);
			}
		}
	}
	return EmptyCells;
}
TArray<FTacCoordinates> UGridDataManager::GetOccupiedCells(ETacGridLayer Layer, UBattleTeam* Team) const
{
	TArray<FTacCoordinates> OccupiedCells;
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
				OccupiedCells.Add(FTacCoordinates(Row, Col, Layer));
			}
		}
	}
	return OccupiedCells;
}
bool UGridDataManager::IsCellOccupied(FTacCoordinates Coords) const
{
	return GetUnit(Coords) != nullptr;
}
TArray<FTacCoordinates> UGridDataManager::GetValidPlacementCells(ETacGridLayer Layer) const
{
	TArray<FTacCoordinates> PlacementCells = GetEmptyCells(Layer);
	PlacementCells.RemoveAll([this](const FTacCoordinates& Coords)
	{
		return IsRestrictedCell(Coords);
	});
	return PlacementCells;
}
void UGridDataManager::PushCorpse(AUnit* Unit, FTacCoordinates Coords)
{
	if (!Unit || !Coords.IsValidCell())
	{
		return;
	}
	// Corpses only exist on ground layer
	if (Coords.Row >= GroundLayer.Num() || Coords.Col >= GroundLayer[Coords.Row].CorpseStacks.Num())
	{
		return;
	}
	FVector WorldLocation = FTacCoordinates::CellToWorldLocation(Coords.Row, Coords.Col, ETacGridLayer::Ground, GridWorldLocation, Grid->GetCellSize(), Grid->GetAirLayerHeight());
	GroundLayer[Coords.Row].CorpseStacks[Coords.Col].Push(Unit, WorldLocation);
}
AUnit* UGridDataManager::GetTopCorpse(FTacCoordinates Coords) const
{
	if (!Coords.IsValidCell())
	{
		return nullptr;
	}
	if (Coords.Row >= GroundLayer.Num() || Coords.Col >= GroundLayer[Coords.Row].CorpseStacks.Num())
	{
		return nullptr;
	}
	return GroundLayer[Coords.Row].CorpseStacks[Coords.Col].Top();
}
AUnit* UGridDataManager::PopCorpse(FTacCoordinates Coords)
{
	if (!Coords.IsValidCell())
	{
		return nullptr;
	}
	if (Coords.Row >= GroundLayer.Num() || Coords.Col >= GroundLayer[Coords.Row].CorpseStacks.Num())
	{
		return nullptr;
	}
	return GroundLayer[Coords.Row].CorpseStacks[Coords.Col].Pop();
}
bool UGridDataManager::HasCorpses(FTacCoordinates Coords) const
{
	if (!Coords.IsValidCell())
	{
		return false;
	}
	if (Coords.Row >= GroundLayer.Num() || Coords.Col >= GroundLayer[Coords.Row].CorpseStacks.Num())
	{
		return false;
	}
	return !GroundLayer[Coords.Row].CorpseStacks[Coords.Col].IsEmpty();
}
const TArray<TObjectPtr<AUnit>>& UGridDataManager::GetCorpseStack(FTacCoordinates Coords) const
{
	static const TArray<TObjectPtr<AUnit>> EmptyStack;
	if (!Coords.IsValidCell())
	{
		return EmptyStack;
	}
	if (Coords.Row >= GroundLayer.Num() || Coords.Col >= GroundLayer[Coords.Row].CorpseStacks.Num())
	{
		return EmptyStack;
	}
	return GroundLayer[Coords.Row].CorpseStacks[Coords.Col].GetAll();
}

TArray<AUnit*> UGridDataManager::GetUnitsInCells(const TArray<FTacCoordinates>& CellCoords, ETacGridLayer Layer) const
{
	TMap<FGuid, TObjectPtr<AUnit>> UniqueUnits;
	const TArray<FGridRow>& LayerArray = GetLayer(Layer);

	for (const FTacCoordinates& Coords : CellCoords)
	{
		if (Coords.IsValidCell() && Coords.Row < LayerArray.Num() && Coords.Col < LayerArray[Coords.Row].Cells.Num())
		{
			const TObjectPtr<AUnit>& Unit = LayerArray[Coords.Row].Cells[Coords.Col];
			if (Unit)
			{
				UniqueUnits.Add(Unit->GetGUID(), Unit);
			}
		}
	}

	TArray<AUnit*> Result;
	UniqueUnits.GenerateValueArray(Result);
	return Result;
}

bool UGridDataManager::IsMultiCellUnit(const AUnit* Unit) const
{
	if (!Unit)
	{
		return false;
	}
	return MultiCellUnits.Contains(Unit->GetGUID());
}

const FMultiCellUnitData* UGridDataManager::GetMultiCellData(const AUnit* Unit) const
{
	if (!Unit)
	{
		return nullptr;
	}
	return MultiCellUnits.Find(Unit->GetGUID());
}

TArray<AUnit*> UGridDataManager::GetAllAliveUnits() const
{
	TMap<FGuid, TObjectPtr<AUnit>> UniqueUnits;

	for (const FGridRow& Row : GroundLayer)
	{
		for (const TObjectPtr<AUnit>& Unit : Row.Cells)
		{
			if (Unit)
			{
				UniqueUnits.Add(Unit->GetGUID(), Unit);
			}
		}
	}

	for (const FGridRow& Row : AirLayer)
	{
		for (const TObjectPtr<AUnit>& Unit : Row.Cells)
		{
			if (Unit)
			{
				UniqueUnits.Add(Unit->GetGUID(), Unit);
			}
		}
	}

	TArray<AUnit*> Result;
	UniqueUnits.GenerateValueArray(Result);
	return Result;
}

TArray<AUnit*> UGridDataManager::GetAllDeadUnits() const
{
	TMap<FGuid, TObjectPtr<AUnit>> UniqueCorpses;

	for (const FGridRow& Row : GroundLayer)
	{
		for (const FCorpseStack& Stack : Row.CorpseStacks)
		{
			for (const TObjectPtr<AUnit>& Corpse : Stack.GetAll())
			{
				if (Corpse)
				{
					UniqueCorpses.Add(Corpse->GetGUID(), Corpse);
				}
			}
		}
	}

	for (const FGridRow& Row : AirLayer)
	{
		for (const FCorpseStack& Stack : Row.CorpseStacks)
		{
			for (const TObjectPtr<AUnit>& Corpse : Stack.GetAll())
			{
				if (Corpse)
				{
					UniqueCorpses.Add(Corpse->GetGUID(), Corpse);
				}
			}
		}
	}

	TArray<AUnit*> Result;
	UniqueCorpses.GenerateValueArray(Result);
	return Result;
}

TArray<AUnit*> UGridDataManager::GetAllUnits() const
{
	TMap<FGuid, TObjectPtr<AUnit>> AllUnits;

	TArray<AUnit*> AliveUnits = GetAllAliveUnits();
	for (AUnit* Unit : AliveUnits)
	{
		if (Unit)
		{
			AllUnits.Add(Unit->GetGUID(), Unit);
		}
	}

	TArray<AUnit*> DeadUnits = GetAllDeadUnits();
	for (AUnit* Unit : DeadUnits)
	{
		if (Unit)
		{
			AllUnits.Add(Unit->GetGUID(), Unit);
		}
	}

	TArray<AUnit*> Result;
	AllUnits.GenerateValueArray(Result);
	return Result;
}

bool UGridDataManager::IsBothTeamsAnyUnitAlive() const
{
	return AttackerTeam->IsAnyUnitAlive() && DefenderTeam->IsAnyUnitAlive();
}
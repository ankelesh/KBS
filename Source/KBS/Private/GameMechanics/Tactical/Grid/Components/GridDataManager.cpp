#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectComponent.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"

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
		bPlayerIsAttacker = Grid->GetPlayerTeamSide() == ETeamSide::Attacker;
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

	LayerArray[Coords.Row].Cells[Coords.Col] = Unit;
	Unit->SetActorLocation(Coords.ToWorldLocation(GridWorldLocation, Grid->GetCellSize(), Grid->GetAirLayerHeight()));

	const int32 UnitSize = Unit->GetUnitDefinition()->UnitSize;
	const bool bOnFlank = Coords.IsFlankCell();

	// Flank overrides orientation and compresses 2-cell units (no ExtraCell on flank).
	const EUnitOrientation Orientation = bOnFlank
		? Coords.GetFlankOrientation()
		: FUnitGridMetadata::DefaultOrientationForTeam(Unit->GetTeamSide());

	FTacCoordinates ExtraCell = FTacCoordinates::Invalid();
	if (UnitSize > 1 && !bOnFlank)
	{
		const FTacCoordinates Candidate = GetExtraCellCoords(Coords, Orientation);
		if (Candidate.IsValidCell()
			&& Candidate.Row < LayerArray.Num()
			&& Candidate.Col < LayerArray[Candidate.Row].Cells.Num()
			&& LayerArray[Candidate.Row].Cells[Candidate.Col] == nullptr)
		{
			LayerArray[Candidate.Row].Cells[Candidate.Col] = Unit;
			ExtraCell = Candidate;
		}
	}

	Unit->GridMetadata = FUnitGridMetadata(Coords, Unit->GetTeamSide(), true, bOnFlank,
		Orientation, ExtraCell, UnitSize);
	Unit->NotifyOrientationChanged();
	UE_LOG(LogTacGrid, Log, TEXT("PlaceUnit: %s -> [%d,%d]"), *Unit->GetLogName(), Coords.Row, Coords.Col);
	return true;
}

// Convenience overload (delegates to primary)
bool UGridDataManager::PlaceUnit(AUnit* Unit, int32 Row, int32 Col, ETacGridLayer Layer)
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

	if (Unit->GridMetadata.HasExtraCell())
	{
		const FTacCoordinates Extra = Unit->GridMetadata.ExtraCell;
		TArray<FGridRow>& ExtraLayer = GetLayer(Extra.Layer);
		if (Extra.Row < ExtraLayer.Num() && Extra.Col < ExtraLayer[Extra.Row].Cells.Num())
		{
			ExtraLayer[Extra.Row].Cells[Extra.Col] = nullptr;
		}
	}

	LayerArray[Coords.Row].Cells[Coords.Col] = nullptr;

	Unit->GridMetadata = FUnitGridMetadata(Unit->GridMetadata.Coords, Unit->GridMetadata.Team, false, false,
		Unit->GridMetadata.Orientation, FTacCoordinates::Invalid(), Unit->GridMetadata.UnitSize);
	UE_LOG(LogTacGrid, Log, TEXT("RemoveUnit: %s from [%d,%d]"), *Unit->GetLogName(), Coords.Row, Coords.Col);
	return true;
}

// Convenience overload (delegates to primary)
bool UGridDataManager::RemoveUnit(int32 Row, int32 Col, ETacGridLayer Layer)
{
	return RemoveUnit(FTacCoordinates(Row, Col, Layer));
}

bool UGridDataManager::RemoveUnit(AUnit* Unit)
{
	checkf(Unit, TEXT("RemoveUnit: Unit must not be null"));
	return RemoveUnit(Unit->GridMetadata.Coords);
}

TArray<FGridRow>& UGridDataManager::GetLayer(ETacGridLayer Layer)
{
	return Layer == ETacGridLayer::Ground ? GroundLayer : AirLayer;
}
const TArray<FGridRow>& UGridDataManager::GetLayer(ETacGridLayer Layer) const
{
	return Layer == ETacGridLayer::Ground ? GroundLayer : AirLayer;
}
bool UGridDataManager::IsUnitOnFlank(const AUnit* Unit) const
{
	checkf(Unit, TEXT("IsUnitOnFlank: Unit must not be null"));
	return Unit->GridMetadata.bOnFlank;
}

void UGridDataManager::SetUnitOrientation(AUnit* Unit, EUnitOrientation Orientation)
{
	checkf(Unit, TEXT("SetUnitOrientation: Unit must not be null"));
	Unit->GridMetadata.Orientation = Orientation;
	Unit->NotifyOrientationChanged();
}

void UGridDataManager::SetUnitOnFlank(AUnit* Unit, bool bOnFlank)
{
	checkf(Unit, TEXT("SetUnitOnFlank: Unit must not be null"));
	Unit->GridMetadata.bOnFlank = bOnFlank;
	if (bOnFlank)
	{
		checkf(Unit->GridMetadata.Coords.IsFlankCell(), TEXT("SetUnitOnFlank: unit coords are not a flank cell"));
		SetUnitOrientation(Unit, Unit->GridMetadata.Coords.GetFlankOrientation());
	}
	else if (!Unit->GridMetadata.IsMultiCell())
	{
		SetUnitOrientation(Unit, FUnitGridMetadata::DefaultOrientationForTeam(Unit->GridMetadata.Team));
	}
	// 2-cell leaving flank: orientation unchanged by contract
	UE_LOG(LogTacGrid, Log, TEXT("SetUnitOnFlank: %s -> %s"), *Unit->GetLogName(), bOnFlank ? TEXT("Flank") : TEXT("Center"));
}
FVector UGridDataManager::GetCellWorldLocation(FTacCoordinates Coords) const
{
	if (!Grid || !Grid->Config)
	{
		return FVector::ZeroVector;
	}
	return FTacCoordinates::CellToWorldLocation(Coords.Row, Coords.Col, Coords.Layer, GridWorldLocation, Grid->Config->CellSize, Grid->Config->AirLayerHeight);
}
UBattleTeam* UGridDataManager::GetTeamBySide(ETeamSide Side) const
{
	return Side == ETeamSide::Attacker ? AttackerTeam : DefenderTeam;
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
		return Coords.IsRestrictedCell();
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
	UE_LOG(LogTacGrid, Log, TEXT("PushCorpse: %s -> [%d,%d]"), *Unit->GetLogName(), Coords.Row, Coords.Col);
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
	AUnit* Corpse = GroundLayer[Coords.Row].CorpseStacks[Coords.Col].Pop();
	UE_LOG(LogTacGrid, Log, TEXT("PopCorpse: %s from [%d,%d]"), *Corpse->GetLogName(), Coords.Row, Coords.Col);
	return Corpse;
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
				UniqueUnits.Add(Unit->GetUnitID(), Unit);
			}
		}
	}

	TArray<TObjectPtr<AUnit>> Result;
	UniqueUnits.GenerateValueArray(Result);
	return Result;
}

void UGridDataManager::FilterCells(ETacGridLayer Layer, TFunctionRef<bool(FTacCoordinates)> Predicate, TArray<FTacCoordinates>& OutCells) const
{
	for (int32 Row = 0; Row < FGridConstants::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridConstants::GridSize; ++Col)
		{
			FTacCoordinates Coords(Row, Col, Layer);
			if (Predicate(Coords))
			{
				OutCells.Add(Coords);
			}
		}
	}
}

TArray<AUnit*> UGridDataManager::GetUnits(EUnitQuerySource Sources) const
{
	TMap<FGuid, TObjectPtr<AUnit>> Result;

	if (EnumHasAnyFlags(Sources, EUnitQuerySource::OnField))
	{
		for (const TArray<FGridRow>* Layer : { &GroundLayer, &AirLayer })
			for (const FGridRow& Row : *Layer)
				for (const TObjectPtr<AUnit>& Unit : Row.Cells)
					if (Unit) Result.Add(Unit->GetUnitID(), Unit);
	}
	if (EnumHasAnyFlags(Sources, EUnitQuerySource::OffField))
	{
		for (const auto& Pair : OffFieldUnits)
			if (Pair.Value) Result.Add(Pair.Key, Pair.Value);
	}
	if (EnumHasAnyFlags(Sources, EUnitQuerySource::Corpses))
	{
		for (const TArray<FGridRow>* Layer : { &GroundLayer, &AirLayer })
			for (const FGridRow& Row : *Layer)
				for (const FCorpseStack& Stack : Row.CorpseStacks)
					for (const TObjectPtr<AUnit>& Corpse : Stack.GetAll())
						if (Corpse) Result.Add(Corpse->GetUnitID(), Corpse);
	}

	TArray<TObjectPtr<AUnit>> Out;
	Result.GenerateValueArray(Out);
	return Out;
}

TArray<AUnit*> UGridDataManager::GetUnits(EUnitQuerySource Sources, TFunctionRef<bool(const AUnit*)> Predicate) const
{
	TArray<AUnit*> All = GetUnits(Sources);
	All.RemoveAll([&](const AUnit* U) { return !Predicate(U); });
	return All;
}

AUnit* UGridDataManager::SpawnUnit(TSubclassOf<AUnit> UnitClass, UUnitDefinition* Definition,
                                   FTacCoordinates Cell, UBattleTeam* Team)
{
	FVector SpawnLocation = GetCellWorldLocation(Cell);
	AUnit* NewUnit = Grid->GetWorld()->SpawnActorDeferred<AUnit>(
		UnitClass,
		FTransform(SpawnLocation),
		nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (!NewUnit) return nullptr;

	if (Definition)
	{
		NewUnit->SetUnitDefinition(Definition);
	}
	Team->AddUnit(NewUnit);
	NewUnit->SetTeamSide(Team->GetTeamSide());
	NewUnit->FinishSpawning(FTransform(SpawnLocation));

	if (!PlaceUnit(NewUnit, Cell))
	{
		Team->RemoveUnit(NewUnit);
		NewUnit->Destroy();
		return nullptr;
	}
	return NewUnit;
}

void UGridDataManager::PlaceUnitOffField(AUnit* Unit)
{
	const bool bFleeing = Unit->GetStats().Status.IsFleeing();
	PlaceUnitOffField(Unit, bFleeing, bFleeing);
}

void UGridDataManager::PlaceUnitOffField(AUnit* Unit, bool bClearEffects, bool bUnsubscribeAbilities)
{
	checkf(Unit->GridMetadata.IsOnField(), TEXT("PlaceUnitOffField: %s is not on the grid"), *Unit->GetLogName());

	RemoveUnit(Unit);
	Unit->GridMetadata.Coords = FTacCoordinates::Invalid();

	if (bClearEffects)
	{
		Unit->EffectManager->ClearAllEffects();
	}
	if (bUnsubscribeAbilities)
	{
		Unit->GetAbilityInventory()->UnregisterPassives();
	}

	OffFieldUnits.Add(Unit->GetUnitID(), Unit);
	UE_LOG(LogTacGrid, Log, TEXT("PlaceUnitOffField: %s"), *Unit->GetLogName());
}

bool UGridDataManager::ReturnUnitToField(const FGuid& UnitID, FTacCoordinates TargetCoords)
{
	checkf(OffFieldUnits.Contains(UnitID), TEXT("ReturnUnitToField: unit not in off-field container"));

	if (!TargetCoords.IsValidCell() || IsCellOccupied(TargetCoords))
	{
		return false;
	}

	AUnit* Unit = OffFieldUnits[UnitID];
	OffFieldUnits.Remove(UnitID);
	PlaceUnit(Unit, TargetCoords);
	UE_LOG(LogTacGrid, Log, TEXT("ReturnUnitToField: %s -> [%d,%d]"), *Unit->GetLogName(), TargetCoords.Row, TargetCoords.Col);
	return true;
}

bool UGridDataManager::IsUnitOffField(const AUnit* Unit) const
{
	return OffFieldUnits.Contains(Unit->GetUnitID());
}

TArray<AUnit*> UGridDataManager::GetOffFieldUnits() const
{
	TArray<TObjectPtr<AUnit>> Values;
	OffFieldUnits.GenerateValueArray(Values);
	return Values;
}

void UGridDataManager::RemoveUnitFromGrid(AUnit* Unit)
{
	if (Unit->GridMetadata.IsOnField())
	{
		RemoveUnit(Unit);
	}
	UBattleTeam* Team = GetTeamBySide(Unit->GetGridMetadata().Team);
	if (Team)
	{
		Team->RemoveUnit(Unit);
	}
}

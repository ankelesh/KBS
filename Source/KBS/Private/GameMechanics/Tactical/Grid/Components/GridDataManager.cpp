#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"

void UGridDataManager::Initialize(ATacBattleGrid* InGrid, UBattleTeam* InAttackerTeam, UBattleTeam* InDefenderTeam)
{
	Grid = InGrid;
	AttackerTeam = InAttackerTeam;
	DefenderTeam = InDefenderTeam;

	if (Grid)
	{
		GridWorldLocation = Grid->GetActorLocation();
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
	Unit->SetActorLocation(FGridCoordinates::CellToWorldLocation(Row, Col, Layer, Grid->GetActorLocation()));
	Unit->SetActorEnableCollision(true);

	// Enable click events on all unit meshes
	if (Unit->VisualsComponent)
	{
		for (USceneComponent* MeshComp : Unit->VisualsComponent->GetAllMeshComponents())
		{
			if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(MeshComp))
			{
				PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				PrimComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
			}
		}
	}

	// Subscribe to unit click events
	Unit->OnUnitClicked.AddDynamic(TacBattleGrid, &ATacBattleGrid::HandleUnitClicked);
	UE_LOG(LogTemp, Log, TEXT("Grid subscribed to unit at [%d,%d]"), Row, Col);

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

	// Unsubscribe from unit click events
	Unit->OnUnitClicked.RemoveDynamic(TacBattleGrid, &ATacBattleGrid::HandleUnitClicked);
	UE_LOG(LogTemp, Log, TEXT("Grid unsubscribed from unit at [%d,%d]"), Row, Col);

	// Clean up external state tracking to prevent memory leaks
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

	// Search ground layer
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

	// Search air layer
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
	return Grid ? Grid->IsValidCell(Row, Col, Layer) : false;
}

bool UGridDataManager::IsFlankCell(int32 Row, int32 Col) const
{
	return Grid ? Grid->IsFlankCell(Row, Col) : false;
}

bool UGridDataManager::IsRestrictedCell(int32 Row, int32 Col) const
{
	return Grid ? Grid->IsRestrictedCell(Row, Col) : false;
}

FVector UGridDataManager::GetCellWorldLocation(int32 Row, int32 Col, EBattleLayer Layer) const
{
	return Grid ? Grid->GetCellWorldLocation(Row, Col, Layer) : FVector::ZeroVector;
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

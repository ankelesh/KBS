#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"

void UGridDataManager::Initialize()
{
	GroundLayer.SetNum(FGridCoordinates::GridSize);
	AirLayer.SetNum(FGridCoordinates::GridSize);
}

bool UGridDataManager::PlaceUnit(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer, ATacBattleGrid* Grid)
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

	// Enable click events on unit mesh
	if (Unit->MeshComponent)
	{
		Unit->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Unit->MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

	// Subscribe to unit click events
	Unit->OnUnitClicked.AddDynamic(Grid, &ATacBattleGrid::HandleUnitClicked);
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

bool UGridDataManager::RemoveUnit(int32 Row, int32 Col, EBattleLayer Layer, ATacBattleGrid* Grid)
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
	Unit->OnUnitClicked.RemoveDynamic(Grid, &ATacBattleGrid::HandleUnitClicked);
	UE_LOG(LogTemp, Log, TEXT("Grid unsubscribed from unit at [%d,%d]"), Row, Col);

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

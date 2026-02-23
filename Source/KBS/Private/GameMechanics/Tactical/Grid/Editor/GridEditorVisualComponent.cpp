#include "GameMechanics/Tactical/Grid/Editor/GridEditorVisualComponent.h"

#if WITH_EDITOR

#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameplayTypes/GridCoordinates.h"
#include "DrawDebugHelpers.h"

UGridEditorVisualComponent::UGridEditorVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

ATacBattleGrid* UGridEditorVisualComponent::GetGrid() const
{
	return Cast<ATacBattleGrid>(GetOwner());
}

void UGridEditorVisualComponent::DrawPreview()
{
	ATacBattleGrid* Grid = GetGrid();
	if (!Grid || !Grid->GetWorld())
	{
		return;
	}

	DrawUnitPlacements();
	DrawGridCells();
}

void UGridEditorVisualComponent::DrawUnitPlacements()
{
	ATacBattleGrid* Grid = GetGrid();
	if (!Grid || !Grid->Config)
	{
		return;
	}

	const float CellSize = Grid->Config->CellSize;
	const float AirLayerHeight = Grid->Config->AirLayerHeight;
	const FVector GridOrigin = Grid->GetActorLocation();

	for (const FUnitPlacement& Placement : Grid->EditorUnitPlacements)
	{
		if (Placement.UnitClass)
		{
			FVector PrimaryCellCenter = FTacCoordinates::CellToWorldLocation(Placement.Row, Placement.Col, Placement.Layer, GridOrigin, CellSize, AirLayerHeight);
			PrimaryCellCenter.Z += 50;
			FColor TeamColor = Placement.bIsAttacker ? FColor::Red : FColor::Green;
			DrawDebugSphere(GetWorld(), PrimaryCellCenter, 50.f, 8, TeamColor, true, -1.f, 0, 5.f);
		}
	}
}

void UGridEditorVisualComponent::DrawGridCells()
{
	ATacBattleGrid* Grid = GetGrid();
	if (!Grid || !Grid->Config)
	{
		return;
	}

	const float CellSize = Grid->Config->CellSize;
	const float AirLayerHeight = Grid->Config->AirLayerHeight;
	const FVector GridOrigin = Grid->GetActorLocation();

	for (int32 Row = 0; Row < FGridConstants::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridConstants::GridSize; ++Col)
		{
			if (FTacCoordinates::IsRestrictedCell(Row, Col))
			{
				continue;
			}
			const bool bIsFlank = FTacCoordinates::IsFlankCell(Row, Col);
			FVector CellCenter = FTacCoordinates::CellToWorldLocation(Row, Col, ETacGridLayer::Ground, GridOrigin, CellSize, AirLayerHeight);
			FColor GroundColor = bIsFlank ? FColor(138, 43, 226) : FColor::Green;
			DrawDebugBox(GetWorld(), CellCenter, FVector(CellSize * 0.5f, CellSize * 0.5f, 5.0f),
				GroundColor, true, -1.0f, 0, 2.0f);
			CellCenter = FTacCoordinates::CellToWorldLocation(Row, Col, ETacGridLayer::Air, GridOrigin, CellSize, AirLayerHeight);
			DrawDebugBox(GetWorld(), CellCenter, FVector(CellSize * 0.5f, CellSize * 0.5f, 5.0f),
				FColor::Cyan, true, -1.0f, 0, 1.0f);
		}
	}
}

#endif // WITH_EDITOR

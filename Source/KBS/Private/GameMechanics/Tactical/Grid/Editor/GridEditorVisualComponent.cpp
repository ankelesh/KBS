#include "GameMechanics/Tactical/Grid/Editor/GridEditorVisualComponent.h"

#if WITH_EDITOR

#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/LargeUnit.h"
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

			const bool bIsLargeUnit = Placement.UnitClass->IsChildOf(ALargeUnit::StaticClass());

			if (bIsLargeUnit)
			{
				bool bIsFlank = FTacCoordinates::IsFlankCell(Placement.Row, Placement.Col);
				ETeamSide TeamSide = Placement.bIsAttacker ? ETeamSide::Attacker : ETeamSide::Defender;
				FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(Placement.Row, Placement.Col, bIsFlank, TeamSide);

				if (FTacCoordinates::IsValidCell(SecondaryCell.X, SecondaryCell.Y))
				{
					FVector SecondaryCellCenter = FTacCoordinates::CellToWorldLocation(SecondaryCell.X, SecondaryCell.Y, Placement.Layer, GridOrigin, CellSize, AirLayerHeight);
					SecondaryCellCenter.Z += 50;

					DrawDebugBox(GetWorld(), PrimaryCellCenter, FVector(40.f, 40.f, 40.f), TeamColor, true, -1.f, 0, 5.f);
					DrawDebugBox(GetWorld(), SecondaryCellCenter, FVector(40.f, 40.f, 40.f), TeamColor.WithAlpha(180), true, -1.f, 0, 5.f);
					DrawDebugLine(GetWorld(), PrimaryCellCenter, SecondaryCellCenter, TeamColor, true, -1.f, 0, 8.f);

					FVector MidPoint = (PrimaryCellCenter + SecondaryCellCenter) * 0.5f;
					DrawDebugString(GetWorld(), MidPoint + FVector(0, 0, 30), TEXT("2-CELL"), nullptr, FColor::Yellow, -1.f, true, 1.2f);
				}
			}
			else
			{
				DrawDebugSphere(GetWorld(), PrimaryCellCenter, 50.f, 8, TeamColor, true, -1.f, 0, 5.f);
			}
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
			FVector CellCenter = FTacCoordinates::CellToWorldLocation(Row, Col, EBattleLayer::Ground, GridOrigin, CellSize, AirLayerHeight);
			FColor GroundColor = bIsFlank ? FColor(138, 43, 226) : FColor::Green;
			DrawDebugBox(GetWorld(), CellCenter, FVector(CellSize * 0.5f, CellSize * 0.5f, 5.0f),
				GroundColor, true, -1.0f, 0, 2.0f);
			CellCenter = FTacCoordinates::CellToWorldLocation(Row, Col, EBattleLayer::Air, GridOrigin, CellSize, AirLayerHeight);
			DrawDebugBox(GetWorld(), CellCenter, FVector(CellSize * 0.5f, CellSize * 0.5f, 5.0f),
				FColor::Cyan, true, -1.0f, 0, 1.0f);
		}
	}
}

#endif // WITH_EDITOR

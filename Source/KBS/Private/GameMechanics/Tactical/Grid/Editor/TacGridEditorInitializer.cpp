#include "GameMechanics/Tactical/Grid/Editor/TacGridEditorInitializer.h"

#if WITH_EDITOR

#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameplayTypes/GridCoordinates.h"

UTacGridEditorInitializer::UTacGridEditorInitializer()
{
	PrimaryComponentTick.bCanEverTick = false;
}

ATacBattleGrid* UTacGridEditorInitializer::GetGrid() const
{
	return Cast<ATacBattleGrid>(GetOwner());
}

void UTacGridEditorInitializer::SpawnAndPlaceUnits()
{
	ATacBattleGrid* Grid = GetGrid();
	if (!Grid || !Grid->GetDataManager())
	{
		return;
	}

	for (const FUnitPlacement& Placement : Grid->EditorUnitPlacements)
	{
		if (!Placement.UnitClass)
		{
			continue;
		}
		if (!FTacCoordinates::IsValidCell(Placement.Row, Placement.Col))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid cell [%d,%d] for unit placement"), Placement.Row, Placement.Col);
			continue;
		}
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Grid;
		AUnit* NewUnit = GetWorld()->SpawnActorDeferred<AUnit>(Placement.UnitClass, FTransform::Identity, Grid, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (NewUnit)
		{
			if (Placement.Definition)
			{
				NewUnit->SetUnitDefinition(Placement.Definition);
			}
			NewUnit->FinishSpawning(FTransform::Identity);
			const bool bPlaced = Grid->GetDataManager()->PlaceUnit(NewUnit, Placement.Row, Placement.Col, Placement.Layer);
			if (bPlaced)
			{
				Grid->SpawnedUnits.Add(NewUnit);
				UBattleTeam* Team = Placement.bIsAttacker ? Grid->GetDataManager()->GetAttackerTeam() : Grid->GetDataManager()->GetDefenderTeam();
				Team->AddUnit(NewUnit);
				NewUnit->SetTeamSide(Team->GetTeamSide());

				NewUnit->SetActorRotation(NewUnit->GetGridMetadata().Rotation);
				UE_LOG(LogTemp, Log, TEXT("Placed unit at [%d,%d] on layer %d"), Placement.Row, Placement.Col, (int32)Placement.Layer);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to place unit at [%d,%d]"), Placement.Row, Placement.Col);
				NewUnit->Destroy();
			}
		}
	}
}

void UTacGridEditorInitializer::SetupUnitEventBindings()
{
	SetupUnitsInLayer(ETacGridLayer::Ground);
	SetupUnitsInLayer(ETacGridLayer::Air);
}

void UTacGridEditorInitializer::SetupUnitsInLayer(ETacGridLayer Layer)
{
	ATacBattleGrid* Grid = GetGrid();
	if (!Grid || !Grid->GetDataManager())
	{
		return;
	}

	for (int32 Row = 0; Row < FGridConstants::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridConstants::GridSize; ++Col)
		{
			const TArray<FGridRow>& LayerArray = Grid->GetDataManager()->GetLayer(Layer);
			if (Row < LayerArray.Num() && Col < LayerArray[Row].Cells.Num() && LayerArray[Row].Cells[Col])
			{
				AUnit* Unit = LayerArray[Row].Cells[Col];
				// Skip the extra cell of a 2-cell unit to avoid binding it twice
				if (Unit->GetGridMetadata().ExtraCell == FTacCoordinates(Row, Col, Layer))
				{
					continue;
				}
				BindUnitEvents(Unit, Row, Col, Layer);
			}
		}
	}
}

void UTacGridEditorInitializer::BindUnitEvents(AUnit* Unit, int32 Row, int32 Col, ETacGridLayer Layer)
{
	ATacBattleGrid* Grid = GetGrid();
	if (!Unit || !Grid)
	{
		return;
	}
	Unit->SetActorEnableCollision(true);
	if (UUnitVisualsComponent* VisualsComp = Unit->GetVisualsComponent())
	{
		for (USceneComponent* MeshComp : VisualsComp->GetAllMeshComponents())
		{
			if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(MeshComp))
			{
				PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				PrimComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
			}
		}
	}
	// Unit->OnUnitClicked.AddDynamic(Grid, &ATacBattleGrid::HandleUnitClicked);
	// TODO: Move to subsystem
	// Unit->OnUnitDied.AddDynamic(Grid, &ATacBattleGrid::HandleUnitDied);
	const FString LayerName = (Layer == ETacGridLayer::Ground) ? TEXT("Ground") : TEXT("Air");
	UE_LOG(LogTemp, Log, TEXT("[SUBSCRIBE] Grid subscribed to OnUnitClicked and OnUnitDied for unit '%s' at [%d,%d] %s"),
		*Unit->GetName(), Row, Col, *LayerName);
}

#endif // WITH_EDITOR

#include "GameMechanics/Tactical/Grid/Components/GridRuntimeInitializer.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Components/UnitVisualsComponent.h"
#include "GameplayTypes/GridCoordinates.h"

UGridRuntimeInitializer::UGridRuntimeInitializer()
{
	PrimaryComponentTick.bCanEverTick = false;
}

ATacBattleGrid* UGridRuntimeInitializer::GetGrid() const
{
	return Cast<ATacBattleGrid>(GetOwner());
}

void UGridRuntimeInitializer::SpawnFromBattleSetup(const FTacticalBattleSetup& Setup, TSubclassOf<AUnit> DefaultUnitClass)
{
	ATacBattleGrid* Grid = GetGrid();
	checkf(Grid && Grid->GetConfig(), TEXT("GridRuntimeInitializer: Grid or Config is null"));

	const UGridConfig* Config = Grid->GetConfig();
	SpawnTeamUnits(Setup.AttackerUnits, true,  DefaultUnitClass, Config->AttackerStartCol);
	SpawnTeamUnits(Setup.DefenderUnits, false, DefaultUnitClass, Config->DefenderStartCol);
}

void UGridRuntimeInitializer::SpawnTeamUnits(const TArray<FBattleUnitPlacement>& Units, bool bIsAttacker,
                                              TSubclassOf<AUnit> UnitClass, int32 StartCol)
{
	ATacBattleGrid* Grid = GetGrid();
	UGridDataManager* DM = Grid->GetDataManager();
	checkf(DM, TEXT("GridRuntimeInitializer: DataManager is null"));

	UBattleTeam* Team = bIsAttacker ? DM->GetAttackerTeam() : DM->GetDefenderTeam();

	for (const FBattleUnitPlacement& Placement : Units)
	{
		if (!Placement.Definition) continue;

		const int32 AbsCol = StartCol + Placement.TeamCol;
		const FTacCoordinates Coords(Placement.TeamRow, AbsCol, Placement.Layer);

		AUnit* NewUnit = DM->SpawnUnit(UnitClass, Placement.Definition, Coords, Team);
		if (NewUnit)
		{
			Grid->SpawnedUnits.Add(NewUnit);
		}
	}
}

void UGridRuntimeInitializer::SetupUnitEventBindings()
{
	ATacBattleGrid* Grid = GetGrid();
	if (!Grid) return;

	for (AUnit* Unit : Grid->SpawnedUnits)
	{
		if (!Unit) continue;
		Unit->SetActorEnableCollision(true);
		if (UUnitVisualsComponent* Visuals = Unit->GetVisualsComponent())
		{
			for (USceneComponent* Comp : Visuals->GetAllMeshComponents())
			{
				if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
				{
					Prim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
					Prim->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
				}
			}
		}
	}
}

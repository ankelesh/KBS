#include "GameMechanics/Tactical/Grid/Components/GridRuntimeInitializer.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Units/UnitDefinition.h"
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
	checkf(DefaultUnitClass, TEXT("GridRuntimeInitializer: DefaultUnitClass is not set on grid"));

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

	UTacLogSubsystem* LogSubsystem = GetWorld()->GetSubsystem<UTacLogSubsystem>();
	checkf(LogSubsystem, TEXT("GridRuntimeInitializer: TacLogSubsystem missing"));

	UBattleTeam* Team = bIsAttacker ? DM->GetAttackerTeam() : DM->GetDefenderTeam();

	for (const FBattleUnitPlacement& Placement : Units)
	{
		if (!Placement.Definition) continue;

		const int32 AbsCol = StartCol + Placement.TeamCol;
		const FTacCoordinates Coords(Placement.TeamRow, AbsCol, Placement.Layer);

		AUnit* NewUnit = DM->SpawnUnit(UnitClass, Placement.Definition, Coords, Team);
		if (!NewUnit)
		{
			UE_LOG(LogTemp, Error, TEXT("GridRuntimeInitializer: Failed to spawn %s at [%d,%d]"),
				*Placement.Definition->UnitName, Coords.Row, Coords.Col);
			continue;
		}
		Grid->SpawnedUnits.Add(NewUnit);

		// Unit is spawned and placed atomically by SpawnUnit, so the event opens and closes here
		LogSubsystem->RegisterUnit(NewUnit->GetUnitID(), Placement.Definition->UnitName);
		const FGuid SpawnEventId = LogSubsystem->OpenEvent(ETacLogEventType::UnitSpawn,
			ETacLogEventOrigin::Initiated, NewUnit->GetUnitID(), FGuid());

		FUnitSpawnPayload Payload;
		Payload.SpawnedUnitId    = NewUnit->GetUnitID();
		Payload.SpawnCoords      = Coords;
		Payload.TeamSide         = Team->GetTeamSide();
		Payload.bIsSummon        = false;
		Payload.UnitDefinitionId = Placement.Definition->GetPrimaryAssetId();
		LogSubsystem->CloseEvent(SpawnEventId,
			TInstancedStruct<FTacLogPayload>::Make<FUnitSpawnPayload>(MoveTemp(Payload)));
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

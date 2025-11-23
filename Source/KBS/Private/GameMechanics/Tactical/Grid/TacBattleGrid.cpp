#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "Components/DecalComponent.h"
#include "Components/BoxComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridTargetingComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridHighlightComponent.h"
#include "GameplayTypes/GridCoordinates.h"

ATacBattleGrid::ATacBattleGrid()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetMobility(EComponentMobility::Movable);

	// Create collision box for grid clicks
	GridCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("GridCollision"));
	GridCollision->SetupAttachment(Root);
	GridCollision->SetBoxExtent(FVector(FGridCoordinates::GridSize * FGridCoordinates::CellSize * 0.5f,
										 FGridCoordinates::GridSize * FGridCoordinates::CellSize * 0.5f, 10.0f));
	GridCollision->SetRelativeLocation(FVector(FGridCoordinates::GridSize * FGridCoordinates::CellSize * 0.5f,
												FGridCoordinates::GridSize * FGridCoordinates::CellSize * 0.5f, 0.0f));
	GridCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GridCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	GridCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Create data manager
	DataManager = CreateDefaultSubobject<UGridDataManager>(TEXT("DataManager"));
	DataManager->Initialize();

	// Create component system
	MovementComponent = CreateDefaultSubobject<UGridMovementComponent>(TEXT("MovementComponent"));
	TargetingComponent = CreateDefaultSubobject<UGridTargetingComponent>(TEXT("TargetingComponent"));
	HighlightComponent = CreateDefaultSubobject<UGridHighlightComponent>(TEXT("HighlightComponent"));

	AttackerTeam = CreateDefaultSubobject<UBattleTeam>(TEXT("AttackerTeam"));
	AttackerTeam->SetTeamSide(ETeamSide::Attacker);

	DefenderTeam = CreateDefaultSubobject<UBattleTeam>(TEXT("DefenderTeam"));
	DefenderTeam->SetTeamSide(ETeamSide::Defender);

	// Enable click events on grid
	SetActorEnableCollision(true);
}

void ATacBattleGrid::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	FlushPersistentDebugLines(GetWorld());
	if (bShowPreviewGizmos && GetWorld()->WorldType == EWorldType::Editor)
	{

		for (const FUnitPlacement& Placement : EditorUnitPlacements)
		{
			if (Placement.UnitClass)
			{
				FVector CellCenter = FGridCoordinates::CellToWorldLocation(Placement.Row, Placement.Col, Placement.Layer, GetActorLocation());
				CellCenter.Z += 50;
				DrawDebugSphere(GetWorld(), CellCenter, 50.f, 8, (Placement.bIsAttacker) ? FColor::Red : FColor::Green, true, -1.f, 0, 5.f);
			}
		}
	}

	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridCoordinates::GridSize; ++Col)
		{
			if (FGridCoordinates::IsRestrictedCell(Row, Col))
			{
				continue;
			}

			const bool bIsFlank = FGridCoordinates::IsFlankCell(Row, Col);

			FVector CellCenter = FGridCoordinates::CellToWorldLocation(Row, Col, EBattleLayer::Ground, GetActorLocation());
			FColor GroundColor = bIsFlank ? FColor(138, 43, 226) : FColor::Green;
			DrawDebugBox(GetWorld(), CellCenter, FVector(FGridCoordinates::CellSize * 0.5f, FGridCoordinates::CellSize * 0.5f, 5.0f),
				GroundColor, true, -1.0f, 0, 2.0f);

			CellCenter = FGridCoordinates::CellToWorldLocation(Row, Col, EBattleLayer::Air, GetActorLocation());
			DrawDebugBox(GetWorld(), CellCenter, FVector(FGridCoordinates::CellSize * 0.5f, FGridCoordinates::CellSize * 0.5f, 5.0f),
				FColor::Cyan, true, -1.0f, 0, 1.0f);
		}
	}
#endif
}

void ATacBattleGrid::BeginPlay()
{
	Super::BeginPlay();

	// Initialize components
	MovementComponent->Initialize(this, DataManager);
	TargetingComponent->Initialize(this, DataManager);
	HighlightComponent->Initialize(this, Root, Config->MoveAllowedDecalMaterial, Config->EnemyDecalMaterial);

	// Create decal pool for highlights
	HighlightComponent->CreateDecalPool();

	// Create units from input data
	for (const FUnitPlacement& Placement : EditorUnitPlacements)
	{
		if (!Placement.UnitClass)
		{
			continue;
		}
		if (!FGridCoordinates::IsValidCell(Placement.Row, Placement.Col))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid cell [%d,%d] for unit placement"), Placement.Row, Placement.Col);
			continue;
		}
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		AUnit* NewUnit = GetWorld()->SpawnActor<AUnit>(Placement.UnitClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (NewUnit)
		{
			const bool bPlaced = PlaceUnit(NewUnit, Placement.Row, Placement.Col, Placement.Layer);
			if (bPlaced)
			{
				SpawnedUnits.Add(NewUnit);
				// Assign to team based on row and flank status
				const bool bIsFlank = FGridCoordinates::IsFlankCell(Placement.Row, Placement.Col);
				UBattleTeam* Team = (Placement.bIsAttacker) ? AttackerTeam : DefenderTeam;
				Team->AddUnit(NewUnit);
				// Set rotation based on team (unless on flank, which gets handled separately)
				if (!bIsFlank)
				{
					const float Yaw = (Team == AttackerTeam) ? 0.0f : 180.0f;
					NewUnit->SetActorRotation(FRotator(0.0f, Yaw, 0.0f));
				}
				else
				{
					// Apply flank rotation immediately
					const FRotator FlankRotation = FGridCoordinates::GetFlankRotation(Placement.Row, Placement.Col);
					NewUnit->SetActorRotation(FlankRotation);
					NewUnit->SetOnFlank(true);
					NewUnit->SetOriginalRotation(FRotator(0.0f, (Team == AttackerTeam) ? 0.0f : 180.0f, 0.0f));
				}
				UE_LOG(LogTemp, Log, TEXT("Placed unit at [%d,%d] on layer %d"), Placement.Row, Placement.Col, (int32)Placement.Layer);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to place unit at [%d,%d]"), Placement.Row, Placement.Col);
				NewUnit->Destroy();
			}
		}
	}

	// Subscribe to all existing units' click events
	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridCoordinates::GridSize; ++Col)
		{
			const TArray<FGridRow>& GroundLayer = DataManager->GetLayer(EBattleLayer::Ground);
			if (Row < GroundLayer.Num() && Col < GroundLayer[Row].Cells.Num() && GroundLayer[Row].Cells[Col])
			{
				AUnit* Unit = GroundLayer[Row].Cells[Col];
				Unit->SetActorEnableCollision(true);

				// Enable click events on unit mesh
				if (Unit->MeshComponent)
				{
					Unit->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
					Unit->MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
				}

				Unit->OnUnitClicked.AddDynamic(this, &ATacBattleGrid::HandleUnitClicked);
				UE_LOG(LogTemp, Log, TEXT("Grid subscribed to unit at [%d,%d] Ground"), Row, Col);
			}

			const TArray<FGridRow>& AirLayer = DataManager->GetLayer(EBattleLayer::Air);
			if (Row < AirLayer.Num() && Col < AirLayer[Row].Cells.Num() && AirLayer[Row].Cells[Col])
			{
				AUnit* Unit = AirLayer[Row].Cells[Col];
				Unit->SetActorEnableCollision(true);

				// Enable click events on unit mesh
				if (Unit->MeshComponent)
				{
					Unit->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
					Unit->MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
				}

				Unit->OnUnitClicked.AddDynamic(this, &ATacBattleGrid::HandleUnitClicked);
				UE_LOG(LogTemp, Log, TEXT("Grid subscribed to unit at [%d,%d] Air"), Row, Col);
			}
		}
	}
}

bool ATacBattleGrid::IsValidCell(int32 Row, int32 Col, EBattleLayer Layer) const
{
	return FGridCoordinates::IsValidCell(Row, Col);
}

bool ATacBattleGrid::PlaceUnit(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer)
{
	return DataManager->PlaceUnit(Unit, Row, Col, Layer, this);
}

AUnit* ATacBattleGrid::GetUnit(int32 Row, int32 Col, EBattleLayer Layer) const
{
	return DataManager->GetUnit(Row, Col, Layer);
}

bool ATacBattleGrid::RemoveUnit(int32 Row, int32 Col, EBattleLayer Layer)
{
	return DataManager->RemoveUnit(Row, Col, Layer, this);
}

FVector ATacBattleGrid::GetCellWorldLocation(int32 Row, int32 Col, EBattleLayer Layer) const
{
	return FGridCoordinates::CellToWorldLocation(Row, Col, Layer, GetActorLocation());
}

bool ATacBattleGrid::IsFlankCell(int32 Row, int32 Col) const
{
	return FGridCoordinates::IsFlankCell(Row, Col);
}

bool ATacBattleGrid::IsRestrictedCell(int32 Row, int32 Col) const
{
	return FGridCoordinates::IsRestrictedCell(Row, Col);
}

UBattleTeam* ATacBattleGrid::GetTeamForUnit(AUnit* Unit) const
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

UBattleTeam* ATacBattleGrid::GetEnemyTeam(AUnit* Unit) const
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

TArray<FIntPoint> ATacBattleGrid::GetValidMoveCells(AUnit* Unit) const
{
	return MovementComponent->GetValidMoveCells(Unit);
}

bool ATacBattleGrid::MoveUnit(AUnit* Unit, int32 TargetRow, int32 TargetCol)
{
	return MovementComponent->MoveUnit(Unit, TargetRow, TargetCol);
}

TArray<FIntPoint> ATacBattleGrid::GetValidTargetCells(AUnit* Unit, ETargetReach Reach) const
{
	return TargetingComponent->GetValidTargetCells(Unit, Reach);
}

TArray<AUnit*> ATacBattleGrid::GetValidTargetUnits(AUnit* Unit, ETargetReach Reach) const
{
	return TargetingComponent->GetValidTargetUnits(Unit, Reach);
}

void ATacBattleGrid::SelectUnit(AUnit* Unit)
{
	if (!Unit)
	{
		ClearSelection();
		return;
	}

	SelectedUnit = Unit;

	// Get valid movement cells and show them
	const TArray<FIntPoint> ValidCells = MovementComponent->GetValidMoveCells(Unit);
	HighlightComponent->ShowValidMoves(ValidCells);

	// Get valid target cells and show them
	const TArray<FIntPoint> TargetCells = TargetingComponent->GetValidTargetCells(Unit, ETargetReach::ClosestEnemies);
	HighlightComponent->ShowValidTargets(TargetCells);

	UE_LOG(LogTemp, Warning, TEXT("SelectUnit: Found %d target cells for unit at [%d,%d]"), TargetCells.Num(), Unit->GetGridRow(), Unit->GetGridCol());
	for (const FIntPoint& Cell : TargetCells)
	{
		const int32 Distance = FMath::Abs(Cell.Y - Unit->GetGridRow()) + FMath::Abs(Cell.X - Unit->GetGridCol());
		UE_LOG(LogTemp, Warning, TEXT("  Target at [%d,%d], Distance: %d"), Cell.Y, Cell.X, Distance);
	}
}

bool ATacBattleGrid::TryMoveSelectedUnit(int32 TargetRow, int32 TargetCol)
{
	if (!SelectedUnit)
	{
		UE_LOG(LogTemp, Error, TEXT("TryMoveSelectedUnit: No unit selected!"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("TryMoveSelectedUnit: Attempting to move unit from [%d,%d] to [%d,%d]"),
		SelectedUnit->GetGridRow(), SelectedUnit->GetGridCol(), TargetRow, TargetCol);

	const bool bSuccess = MoveUnit(SelectedUnit, TargetRow, TargetCol);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("TryMoveSelectedUnit: Move successful, clearing selection"));
		ClearSelection();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TryMoveSelectedUnit: Move FAILED!"));
	}

	return bSuccess;
}

void ATacBattleGrid::ClearSelection()
{
	SelectedUnit = nullptr;
	HighlightComponent->ClearHighlights();
}

FIntPoint ATacBattleGrid::GetCellFromWorldLocation(FVector WorldLocation) const
{
	return FGridCoordinates::WorldLocationToCell(WorldLocation, GetActorLocation());
}

void ATacBattleGrid::HandleUnitClicked(AUnit* Unit)
{
	if (!Unit)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Grid received click from unit at [%d,%d]"), Unit->GetGridRow(), Unit->GetGridCol());

	// If we have a selected unit and clicked a different unit, check if it's a valid target
	if (SelectedUnit && SelectedUnit != Unit)
	{
		TArray<AUnit*> ValidTargets = GetValidTargetUnits(SelectedUnit, ETargetReach::ClosestEnemies);
		if (ValidTargets.Contains(Unit))
		{
			// Valid target clicked - trigger conflict
			UnitConflict(SelectedUnit, Unit);
			return;
		}
	}

	// Otherwise select the unit normally
	SelectUnit(Unit);
}

void ATacBattleGrid::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	UE_LOG(LogTemp, Warning, TEXT("Grid NotifyActorOnClicked called!"));

	if (!SelectedUnit)
	{
		UE_LOG(LogTemp, Log, TEXT("Grid clicked but no unit selected"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Grid clicked with unit [%d,%d] selected"), SelectedUnit->GetGridRow(), SelectedUnit->GetGridCol());

	// Get player controller
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("Grid clicked: No PlayerController found!"));
		return;
	}

	// Get mouse world position
	FVector WorldLocation, WorldDirection;
	PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	// Raycast to find grid location
	FHitResult Hit;
	FVector Start = WorldLocation;
	FVector End = Start + WorldDirection * 10000.0f;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		FIntPoint CellCoords = GetCellFromWorldLocation(Hit.Location);
		UE_LOG(LogTemp, Warning, TEXT("Grid clicked at cell [%d,%d], Hit actor: %s"), CellCoords.Y, CellCoords.X, *Hit.GetActor()->GetName());
		TryMoveSelectedUnit(CellCoords.Y, CellCoords.X);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Grid clicked: Raycast did not hit anything!"));
	}
}

void ATacBattleGrid::UnitEntersFlank(AUnit* Unit, int32 Row, int32 Col)
{
	if (!Unit)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("UnitEntersFlank: Unit entered flank cell at [%d,%d]"), Row, Col);

	// Store original rotation before applying flank rotation
	if (!Unit->IsOnFlank())
	{
		Unit->SetOriginalRotation(Unit->GetActorRotation());
	}

	// Apply flank-specific rotation
	const FRotator FlankRotation = FGridCoordinates::GetFlankRotation(Row, Col);
	Unit->SetActorRotation(FlankRotation);
	Unit->SetOnFlank(true);
}

void ATacBattleGrid::UnitExitsFlank(AUnit* Unit)
{
	if (!Unit || !Unit->IsOnFlank())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("UnitExitsFlank: Unit exited flank cell"));

	// Restore original rotation
	Unit->SetActorRotation(Unit->GetOriginalRotation());
	Unit->SetOnFlank(false);
}

void ATacBattleGrid::UnitConflict(AUnit* Attacker, AUnit* Defender)
{
	if (!Attacker || !Defender)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("UnitConflict: Attacker at [%d,%d] vs Defender at [%d,%d]"),
		Attacker->GetGridRow(), Attacker->GetGridCol(),
		Defender->GetGridRow(), Defender->GetGridCol());
	// Future implementation: combat resolution, damage calculation, animations, etc.
}

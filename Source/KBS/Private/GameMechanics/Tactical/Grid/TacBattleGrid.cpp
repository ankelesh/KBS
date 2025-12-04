#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "Components/DecalComponent.h"
#include "Components/BoxComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridTargetingComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridHighlightComponent.h"
#include "GameMechanics/Tactical/Grid/Components/TurnManagerComponent.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Tactical/DamageCalculator.h"
#include "GameplayTypes/CombatTypes.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameplayTypes/AbilityBattleContext.h"
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
	TurnManager = CreateDefaultSubobject<UTurnManagerComponent>(TEXT("TurnManager"));

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
		AUnit* NewUnit = GetWorld()->SpawnActorDeferred<AUnit>(Placement.UnitClass, FTransform::Identity, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (NewUnit)
		{
			// Set definition before BeginPlay so stats initialize correctly
			if (Placement.Definition)
			{
				NewUnit->SetUnitDefinition(Placement.Definition);
			}

			// Trigger BeginPlay (which initializes stats from Definition)
			NewUnit->FinishSpawning(FTransform::Identity);

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

				Unit->OnUnitClicked.AddDynamic(this, &ATacBattleGrid::HandleUnitClicked);
				Unit->OnUnitDied.AddDynamic(this, &ATacBattleGrid::HandleUnitDied);
				UE_LOG(LogTemp, Log, TEXT("Grid subscribed to unit at [%d,%d] Ground"), Row, Col);
			}

			const TArray<FGridRow>& AirLayer = DataManager->GetLayer(EBattleLayer::Air);
			if (Row < AirLayer.Num() && Col < AirLayer[Row].Cells.Num() && AirLayer[Row].Cells[Col])
			{
				AUnit* Unit = AirLayer[Row].Cells[Col];
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

				Unit->OnUnitClicked.AddDynamic(this, &ATacBattleGrid::HandleUnitClicked);
				Unit->OnUnitDied.AddDynamic(this, &ATacBattleGrid::HandleUnitDied);
				UE_LOG(LogTemp, Log, TEXT("Grid subscribed to unit at [%d,%d] Air"), Row, Col);
			}
		}
	}

	// Setup turn manager
	if (TurnManager)
	{
		TurnManager->AttackerTeam = AttackerTeam;
		TurnManager->DefenderTeam = DefenderTeam;

		// Subscribe to action completion events
		OnMovementComplete.AddDynamic(TurnManager, &UTurnManagerComponent::EndCurrentUnitTurn);
		OnAbilityComplete.AddDynamic(TurnManager, &UTurnManagerComponent::EndCurrentUnitTurn);

		// Subscribe to battle end event
		TurnManager->OnBattleEnded.AddDynamic(this, &ATacBattleGrid::HandleBattleEnded);

		// Initialize battle
		TArray<AUnit*> AllBattleUnits;
		AllBattleUnits.Append(AttackerTeam->GetUnits());
		AllBattleUnits.Append(DefenderTeam->GetUnits());

		if (AllBattleUnits.Num() > 0)
		{
			TurnManager->StartBattle(AllBattleUnits);
			UE_LOG(LogTemp, Log, TEXT("Battle started with %d units"), AllBattleUnits.Num());
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

bool ATacBattleGrid::GetUnitPosition(const AUnit* Unit, int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const
{
	return DataManager->GetUnitPosition(Unit, OutRow, OutCol, OutLayer);
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

TArray<FIntPoint> ATacBattleGrid::GetValidTargetCells(AUnit* Unit) const
{
	if (!Unit || !Unit->AbilityInventory)
	{
		return TArray<FIntPoint>();
	}

	UUnitAbilityInstance* CurrentAbility = Unit->AbilityInventory->GetCurrentActiveAbility();
	if (!CurrentAbility)
	{
		return TArray<FIntPoint>();
	}

	ETargetReach Reach = CurrentAbility->GetTargeting();
	return TargetingComponent->GetValidTargetCells(Unit, Reach);
}

TArray<AUnit*> ATacBattleGrid::GetValidTargetUnits(AUnit* Unit) const
{
	if (!Unit || !Unit->AbilityInventory)
	{
		return TArray<AUnit*>();
	}

	UUnitAbilityInstance* CurrentAbility = Unit->AbilityInventory->GetCurrentActiveAbility();
	if (!CurrentAbility)
	{
		return TArray<AUnit*>();
	}

	ETargetReach Reach = CurrentAbility->GetTargeting();
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
	OnCurrentUnitChanged.Broadcast(Unit);

	// Get valid movement cells and show them
	const TArray<FIntPoint> ValidCells = MovementComponent->GetValidMoveCells(Unit);
	HighlightComponent->ShowValidMoves(ValidCells);

	// Get valid target cells and show them
	const TArray<FIntPoint> TargetCells = GetValidTargetCells(Unit);
	HighlightComponent->ShowValidTargets(TargetCells);

	UE_LOG(LogTemp, Warning, TEXT("SelectUnit: Found %d target cells"), TargetCells.Num());
	for (const FIntPoint& Cell : TargetCells)
	{
		UE_LOG(LogTemp, Warning, TEXT("  Target at [%d,%d]"), Cell.Y, Cell.X);
	}
}

bool ATacBattleGrid::TryMoveSelectedUnit(int32 TargetRow, int32 TargetCol)
{
	if (!SelectedUnit)
	{
		UE_LOG(LogTemp, Error, TEXT("TryMoveSelectedUnit: No unit selected!"));
		return false;
	}

	int32 CurrentRow, CurrentCol;
	EBattleLayer CurrentLayer;
	if (GetUnitPosition(SelectedUnit, CurrentRow, CurrentCol, CurrentLayer))
	{
		UE_LOG(LogTemp, Warning, TEXT("TryMoveSelectedUnit: Attempting to move unit from [%d,%d] to [%d,%d]"),
			CurrentRow, CurrentCol, TargetRow, TargetCol);
	}

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

	int32 Row, Col;
	EBattleLayer Layer;
	if (GetUnitPosition(Unit, Row, Col, Layer))
	{
		UE_LOG(LogTemp, Warning, TEXT("Grid received click from unit at [%d,%d]"), Row, Col);
	}

	// If we have a selected unit and clicked a different unit, check if it's a valid target
	if (SelectedUnit && SelectedUnit != Unit)
	{
		TArray<AUnit*> ValidTargets = GetValidTargetUnits(SelectedUnit);
		if (ValidTargets.Contains(Unit))
		{
			// Valid target clicked - trigger ability
			TArray<AUnit*> Targets;
			Targets.Add(Unit);
			AbilityTargetSelected(SelectedUnit, Targets);
			return;
		}
	}

	// Otherwise select the unit normally
	SelectUnit(Unit);
}

void ATacBattleGrid::HandleUnitDied(AUnit* Unit)
{
	if (!Unit || !TurnManager)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Grid received death event from unit %s"), *Unit->GetName());
	TurnManager->RemoveUnitFromQueue(Unit);
}

void ATacBattleGrid::HandleBattleEnded(UBattleTeam* Winner)
{
	if (Winner)
	{
		FString TeamName = (Winner->GetTeamSide() == ETeamSide::Attacker) ? TEXT("Attacker") : TEXT("Defender");
		UE_LOG(LogTemp, Warning, TEXT("=== BATTLE ENDED - Winner: %s Team ==="), *TeamName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("=== BATTLE ENDED - No Winner (Draw?) ==="));
	}
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

	int32 SelectedRow, SelectedCol;
	EBattleLayer SelectedLayer;
	if (GetUnitPosition(SelectedUnit, SelectedRow, SelectedCol, SelectedLayer))
	{
		UE_LOG(LogTemp, Warning, TEXT("Grid clicked with unit [%d,%d] selected"), SelectedRow, SelectedCol);
	}

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
	if (!IsUnitOnFlank(Unit))
	{
		SetUnitOriginalRotation(Unit, Unit->GetActorRotation());
	}

	// Apply flank-specific rotation
	const FRotator FlankRotation = FGridCoordinates::GetFlankRotation(Row, Col);
	Unit->SetActorRotation(FlankRotation);
	SetUnitOnFlank(Unit, true);
}

void ATacBattleGrid::UnitExitsFlank(AUnit* Unit)
{
	if (!Unit || !IsUnitOnFlank(Unit))
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("UnitExitsFlank: Unit exited flank cell"));

	// Restore original rotation
	Unit->SetActorRotation(GetUnitOriginalRotation(Unit));
	SetUnitOnFlank(Unit, false);
}

void ATacBattleGrid::AbilityTargetSelected(AUnit* SourceUnit, const TArray<AUnit*>& Targets)
{
	if (!SourceUnit || Targets.Num() == 0)
	{
		return;
	}

	// Get the unit's current active ability
	if (!SourceUnit->AbilityInventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityTargetSelected: Source unit has no AbilityInventory"));
		return;
	}

	UUnitAbilityInstance* CurrentAbility = SourceUnit->AbilityInventory->GetCurrentActiveAbility();
	if (!CurrentAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityTargetSelected: Source unit has no current active ability"));
		return;
	}

	// Build the ability battle context
	FAbilityBattleContext Context;
	Context.SourceUnit = SourceUnit;
	Context.TargetUnits = Targets;
	Context.Grid = this;

	// Trigger and apply the ability
	if (CurrentAbility->TriggerAbility(Context))
	{
		CurrentAbility->ApplyAbilityEffect(Context);
		UE_LOG(LogTemp, Log, TEXT("AbilityTargetSelected: Applied ability '%s' from unit to %d targets"),
			*CurrentAbility->GetConfig()->AbilityName, Targets.Num());

		// Broadcast ability completion for turn system
		OnAbilityComplete.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityTargetSelected: Ability trigger failed"));
	}
}

bool ATacBattleGrid::IsUnitOnFlank(const AUnit* Unit) const
{
	const bool* bOnFlank = UnitFlankStates.Find(const_cast<AUnit*>(Unit));
	return bOnFlank ? *bOnFlank : false;
}

void ATacBattleGrid::SetUnitOnFlank(AUnit* Unit, bool bOnFlank)
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

FRotator ATacBattleGrid::GetUnitOriginalRotation(const AUnit* Unit) const
{
	const FRotator* Rotation = UnitOriginalRotations.Find(const_cast<AUnit*>(Unit));
	return Rotation ? *Rotation : FRotator::ZeroRotator;
}

void ATacBattleGrid::SetUnitOriginalRotation(AUnit* Unit, const FRotator& Rotation)
{
	UnitOriginalRotations.Add(Unit, Rotation);
}

TArray<AUnit*> ATacBattleGrid::GetPlayerTeamUnits() const
{
	// Assuming AttackerTeam is the player for now
	// You'll adjust this logic when you add team switching
	if (AttackerTeam)
	{
		return AttackerTeam->GetUnits(); // Adjust based on UBattleTeam's API
	}
	return TArray<AUnit*>();
}
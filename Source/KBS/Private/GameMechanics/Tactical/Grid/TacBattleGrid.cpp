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
#include "GameMechanics/Tactical/Grid/Components/AbilityExecutorComponent.h"
#include "GameMechanics/Tactical/Grid/Components/PresentationTrackerComponent.h"
#include "GameMechanics/Tactical/Grid/Components/AIControllerComponent.h"
#include "GameMechanics/Tactical/Grid/Components/TacGridInputRouter.h"
#include "GameMechanics/Tactical/Grid/Components/GridInputLockComponent.h"
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

	// Create data manager (full initialization happens in BeginPlay)
	DataManager = CreateDefaultSubobject<UGridDataManager>(TEXT("DataManager"));

	// Create component system
	MovementComponent = CreateDefaultSubobject<UGridMovementComponent>(TEXT("MovementComponent"));
	TargetingComponent = CreateDefaultSubobject<UGridTargetingComponent>(TEXT("TargetingComponent"));
	HighlightComponent = CreateDefaultSubobject<UGridHighlightComponent>(TEXT("HighlightComponent"));
	TurnManager = CreateDefaultSubobject<UTurnManagerComponent>(TEXT("TurnManager"));
	AbilityExecutor = CreateDefaultSubobject<UAbilityExecutorComponent>(TEXT("AbilityExecutor"));
	PresentationTracker = CreateDefaultSubobject<UPresentationTrackerComponent>(TEXT("PresentationTracker"));
	AIController = CreateDefaultSubobject<UAIControllerComponent>(TEXT("AIController"));
	InputRouter = CreateDefaultSubobject<UTacGridInputRouter>(TEXT("InputRouter"));
	InputLockComponent = CreateDefaultSubobject<UGridInputLockComponent>(TEXT("InputLockComponent"));

	// Enable click events on grid
	SetActorEnableCollision(true);
}

void ATacBattleGrid::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (!bShowPreviewGizmos || GetWorld()->WorldType != EWorldType::Editor)
	{
		return;
	}

	FlushPersistentDebugLines(GetWorld());
	DrawUnitPlacements();
	DrawGridCells();
#endif
}

void ATacBattleGrid::BeginPlay()
{
	Super::BeginPlay();

	InitializeComponents();
	SpawnAndPlaceUnits();
	SetupUnitEventBindings();
	ConfigureTurnManager();
	StartBattle();
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
	return TargetingComponent->GetValidTargetCells(Unit);
}

TArray<AUnit*> ATacBattleGrid::GetValidTargetUnits(AUnit* Unit) const
{
	return TargetingComponent->GetValidTargetUnits(Unit);
}

FIntPoint ATacBattleGrid::GetCellFromWorldLocation(FVector WorldLocation) const
{
	return FGridCoordinates::WorldLocationToCell(WorldLocation, GetActorLocation());
}

void ATacBattleGrid::HandleUnitClicked(AUnit* Unit, FKey ButtonPressed)
{
	if (!Unit)
	{
		return;
	}

	int32 Row, Col;
	EBattleLayer Layer;
	if (GetUnitPosition(Unit, Row, Col, Layer))
	{
		UE_LOG(LogTemp, Warning, TEXT("[HANDLER] Grid received click from unit '%s' at [%d,%d] with button '%s'"),
			*Unit->GetName(), Row, Col, *ButtonPressed.ToString());
	}

	if (ButtonPressed == EKeys::RightMouseButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EVENT] Broadcasting OnDetailsRequested for unit '%s'"), *Unit->GetName());
		OnDetailsRequested.Broadcast(Unit);
	}
}

void ATacBattleGrid::HandleUnitDied(AUnit* Unit)
{
	if (!Unit || !TurnManager)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[HANDLER] Grid received death event from unit '%s'"), *Unit->GetName());
	TurnManager->RemoveUnitFromQueue(Unit);
}

void ATacBattleGrid::HandleBattleEnded(UBattleTeam* Winner)
{
	if (Winner)
	{
		FString TeamName = (Winner->GetTeamSide() == ETeamSide::Attacker) ? TEXT("Attacker") : TEXT("Defender");
		UE_LOG(LogTemp, Warning, TEXT("[HANDLER] === BATTLE ENDED - Winner: %s Team ==="), *TeamName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[HANDLER] === BATTLE ENDED - No Winner (Draw?) ==="));
	}
}

void ATacBattleGrid::HandleUnitTurnStart(AUnit* Unit)
{
	if (!Unit || !TurnManager || !DataManager)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[HANDLER] Grid received unit turn start for '%s'"), *Unit->GetName());

	// Determine which team this unit belongs to
	UBattleTeam* UnitTeam = DataManager->GetTeamForUnit(Unit);
	if (!UnitTeam)
	{
		UE_LOG(LogTemp, Error, TEXT("[HANDLER] Unit '%s' does not belong to any team!"), *Unit->GetName());
		return;
	}

	// Check if this unit belongs to the player-controlled team
	if (UnitTeam->GetTeamSide() == Player1ControlledTeam)
	{
		// Player-controlled unit: TurnManager already set ActiveUnit, just broadcast event
		UE_LOG(LogTemp, Log, TEXT("[HANDLER] Player unit turn started: '%s'"), *Unit->GetName());
		OnCurrentUnitChanged.Broadcast(Unit);

		// Refresh highlights for active unit
		if (HighlightComponent)
		{
			HighlightComponent->ClearHighlights();
			const TArray<FIntPoint> TargetCells = GetValidTargetCells(Unit);
			HighlightComponent->ShowValidTargets(TargetCells);
			const TArray<FIntPoint> ValidCells = GetValidMoveCells(Unit);
			HighlightComponent->ShowValidMoves(ValidCells);
		}
	}
	else
	{
		// AI-controlled unit: execute AI
		UE_LOG(LogTemp, Log, TEXT("[HANDLER] AI unit turn started - executing AI for '%s'"), *Unit->GetName());
		AIController->ExecuteAITurn(Unit);
	}
}

void ATacBattleGrid::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	// Delegate to InputRouter
	if (InputRouter)
	{
		InputRouter->HandleGridClick(ButtonPressed);
	}
}

void ATacBattleGrid::AbilityTargetSelected(AUnit* SourceUnit, const TArray<AUnit*>& Targets)
{
	// Delegate to TurnManager
	if (TurnManager)
	{
		TurnManager->ExecuteAbilityOnTargets(SourceUnit, Targets);
	}
}

void ATacBattleGrid::SwitchAbility(UUnitAbilityInstance* NewAbility)
{
	// Delegate to TurnManager
	if (TurnManager)
	{
		TurnManager->SwitchAbility(NewAbility);
	}
}

void ATacBattleGrid::AbilitySelfExecute(AUnit* SourceUnit, UUnitAbilityInstance* Ability)
{
	// Delegate to TurnManager
	if (TurnManager)
	{
		TurnManager->ExecuteAbilityOnSelf(SourceUnit, Ability);
	}
}

TArray<AUnit*> ATacBattleGrid::GetTeamUnits(bool bIsAttackerTeam) const
{
	if (bIsAttackerTeam)
	{
		return DataManager->GetAttackerTeam()->GetUnits();
	}
	else
	{
		return DataManager->GetDefenderTeam()->GetUnits();
	}
}

UBattleTeam* ATacBattleGrid::GetTeamForUnit(AUnit* Unit) const
{
	return DataManager->GetTeamForUnit(Unit);
}

UBattleTeam* ATacBattleGrid::GetEnemyTeam(AUnit* Unit) const
{
	return DataManager->GetEnemyTeam(Unit);
}


void ATacBattleGrid::SetHoveredUnit(AUnit* Unit)
{
	if (Unit)
	{
		UE_LOG(LogTemp, Log, TEXT("[EVENT] Broadcasting OnUnitHovered for unit '%s'"), *Unit->GetName());
		OnUnitHovered.Broadcast(Unit);
	}
}

UTurnManagerComponent* ATacBattleGrid::GetTurnManager()
{
	return TurnManager;
}

UPresentationTrackerComponent* ATacBattleGrid::GetPresentationTracker() const
{
	return PresentationTracker;
}

UGridDataManager* ATacBattleGrid::GetDataManager()
{
	return DataManager;
}

UGridInputLockComponent* ATacBattleGrid::GetInputLockComponent() const
{
	return InputLockComponent;
}

void ATacBattleGrid::RequestUnitDetails(AUnit* Unit)
{
	if (Unit)
	{
		UE_LOG(LogTemp, Log, TEXT("[BLUEPRINT] RequestUnitDetails called for unit '%s'"), *Unit->GetName());
		OnDetailsRequested.Broadcast(Unit);
	}
}

void ATacBattleGrid::InitializeComponents()
{
	DataManager->Initialize(this);
	MovementComponent->Initialize(this, DataManager);
	TargetingComponent->Initialize(DataManager);

	// Initialize TurnManager with all required component references
	TurnManager->PresentationTracker = PresentationTracker;
	TurnManager->AttackerTeam = DataManager->GetAttackerTeam();
	TurnManager->DefenderTeam = DataManager->GetDefenderTeam();
	TurnManager->AbilityExecutor = AbilityExecutor;
	TurnManager->HighlightComponent = HighlightComponent;
	TurnManager->TargetingComponent = TargetingComponent;
	TurnManager->MovementComponent = MovementComponent;
	TurnManager->InputLockComponent = InputLockComponent;

	AbilityExecutor->Initialize(this, PresentationTracker);
	AIController->Initialize(DataManager, MovementComponent, TargetingComponent, AbilityExecutor, TurnManager);
	HighlightComponent->Initialize(this, Root, Config->MoveAllowedDecalMaterial, Config->EnemyDecalMaterial);
	HighlightComponent->CreateDecalPool();

	// Connect input lock to presentation tracker for auto-locking
	PresentationTracker->SetInputLockComponent(InputLockComponent);

	// Initialize InputRouter
	InputRouter->Initialize(this, DataManager, MovementComponent, TargetingComponent, TurnManager, InputLockComponent);
}

void ATacBattleGrid::SpawnAndPlaceUnits()
{
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
			if (Placement.Definition)
			{
				NewUnit->SetUnitDefinition(Placement.Definition);
			}

			NewUnit->FinishSpawning(FTransform::Identity);

			const bool bPlaced = PlaceUnit(NewUnit, Placement.Row, Placement.Col, Placement.Layer);
			if (bPlaced)
			{
				SpawnedUnits.Add(NewUnit);

				const bool bIsFlank = FGridCoordinates::IsFlankCell(Placement.Row, Placement.Col);
				UBattleTeam* Team = Placement.bIsAttacker ? DataManager->GetAttackerTeam() : DataManager->GetDefenderTeam();
				Team->AddUnit(NewUnit);

				// Set unit's team side for movement orientation
				NewUnit->SetTeamSide(Team->GetTeamSide());

				if (!bIsFlank)
				{
					const float Yaw = (Team == DataManager->GetAttackerTeam()) ? 0.0f : 180.0f;
					NewUnit->SetActorRotation(FRotator(0.0f, Yaw, 0.0f));
				}
				else
				{
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
}

void ATacBattleGrid::SetupUnitEventBindings()
{
	SetupUnitsInLayer(EBattleLayer::Ground);
	SetupUnitsInLayer(EBattleLayer::Air);
}

void ATacBattleGrid::SetupUnitsInLayer(EBattleLayer Layer)
{
	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridCoordinates::GridSize; ++Col)
		{
			const TArray<FGridRow>& LayerArray = DataManager->GetLayer(Layer);
			if (Row < LayerArray.Num() && Col < LayerArray[Row].Cells.Num() && LayerArray[Row].Cells[Col])
			{
				AUnit* Unit = LayerArray[Row].Cells[Col];
				BindUnitEvents(Unit, Row, Col, Layer);
			}
		}
	}
}

void ATacBattleGrid::BindUnitEvents(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer)
{
	if (!Unit)
	{
		return;
	}

	Unit->SetActorEnableCollision(true);

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

	const FString LayerName = (Layer == EBattleLayer::Ground) ? TEXT("Ground") : TEXT("Air");
	UE_LOG(LogTemp, Log, TEXT("[SUBSCRIBE] Grid subscribed to OnUnitClicked and OnUnitDied for unit '%s' at [%d,%d] %s"),
		*Unit->GetName(), Row, Col, *LayerName);
}

void ATacBattleGrid::ConfigureTurnManager()
{
	if (!TurnManager)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SUBSCRIBE] TurnManager subscribed to OnMovementComplete and OnAbilityComplete"));

	TurnManager->OnUnitTurnStart.AddDynamic(this, &ATacBattleGrid::HandleUnitTurnStart);
	UE_LOG(LogTemp, Log, TEXT("[SUBSCRIBE] Grid subscribed to TurnManager->OnUnitTurnStart"));

	TurnManager->OnBattleEnded.AddDynamic(this, &ATacBattleGrid::HandleBattleEnded);
	UE_LOG(LogTemp, Log, TEXT("[SUBSCRIBE] Grid subscribed to TurnManager->OnBattleEnded"));
}

void ATacBattleGrid::StartBattle()
{
	if (!TurnManager)
	{
		return;
	}

	TArray<AUnit*> AllBattleUnits;
	AllBattleUnits.Append(DataManager->GetAttackerTeam()->GetUnits());
	AllBattleUnits.Append(DataManager->GetDefenderTeam()->GetUnits());

	if (AllBattleUnits.Num() > 0)
	{
		TurnManager->StartBattle(AllBattleUnits);
		UE_LOG(LogTemp, Log, TEXT("Battle started with %d units"), AllBattleUnits.Num());
	}
}

void ATacBattleGrid::DrawUnitPlacements()
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

void ATacBattleGrid::DrawGridCells()
{
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
}

bool ATacBattleGrid::GetCellUnderMouse(int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("GetCellUnderMouse: No PlayerController found!"));
		return false;
	}

	FVector WorldLocation, WorldDirection;
	PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	FHitResult Hit;
	FVector Start = WorldLocation;
	FVector End = Start + WorldDirection * 10000.0f;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		FIntPoint CellCoords = GetCellFromWorldLocation(Hit.Location);
		OutRow = CellCoords.Y;
		OutCol = CellCoords.X;

		// Determine layer from hit Z-coordinate
		// Compare distance to Ground and Air layer heights
		FVector GroundLocation = GetCellWorldLocation(OutRow, OutCol, EBattleLayer::Ground);
		FVector AirLocation = GetCellWorldLocation(OutRow, OutCol, EBattleLayer::Air);

		float DistToGround = FMath::Abs(Hit.Location.Z - GroundLocation.Z);
		float DistToAir = FMath::Abs(Hit.Location.Z - AirLocation.Z);

		OutLayer = (DistToGround < DistToAir) ? EBattleLayer::Ground : EBattleLayer::Air;

		return true;
	}

	return false;
}

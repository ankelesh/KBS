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
	UE_LOG(LogTemp, Warning, TEXT("[EVENT] Broadcasting OnCurrentUnitChanged for unit '%s'"), *Unit->GetName());
	OnCurrentUnitChanged.Broadcast(Unit);

	// Subscribe to ability changes
	if (Unit->AbilityInventory)
	{
		Unit->AbilityInventory->OnAbilityEquipped.AddDynamic(this, &ATacBattleGrid::HandleAbilityEquipped);
	}

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
	if (!Unit || !TurnManager)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[HANDLER] Grid received unit turn start for '%s'"), *Unit->GetName());

	// Determine which team this unit belongs to
	UBattleTeam* UnitTeam = GetTeamForUnit(Unit);
	if (!UnitTeam)
	{
		UE_LOG(LogTemp, Error, TEXT("[HANDLER] Unit '%s' does not belong to any team!"), *Unit->GetName());
		return;
	}

	// Check if this unit belongs to the player-controlled team
	if (UnitTeam->GetTeamSide() == Player1ControlledTeam)
	{
		// Player-controlled unit: select it and prepare movement/targeting
		UE_LOG(LogTemp, Log, TEXT("[HANDLER] Player unit turn started - selecting unit '%s'"), *Unit->GetName());
		SelectUnit(Unit);
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

	UE_LOG(LogTemp, Warning, TEXT("Grid NotifyActorOnClicked called!"));

	// Early exit if no unit is selected
	if (!SelectedUnit)
	{
		UE_LOG(LogTemp, Log, TEXT("Grid clicked but no unit selected"));
		return;
	}

	// Get clicked cell with layer information
	int32 ClickedRow, ClickedCol;
	EBattleLayer ClickedLayer;
	if (!GetCellUnderMouse(ClickedRow, ClickedCol, ClickedLayer))
	{
		UE_LOG(LogTemp, Error, TEXT("Grid clicked: Could not determine clicked cell"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Grid clicked at cell [%d,%d] Layer=%d"), ClickedRow, ClickedCol, (int32)ClickedLayer);

	// Get valid target cells and valid move cells
	const TArray<FIntPoint> ValidTargetCells = GetValidTargetCells(SelectedUnit);
	const TArray<FIntPoint> ValidMoveCells = GetValidMoveCells(SelectedUnit);

	FIntPoint ClickedCell(ClickedCol, ClickedRow);

	// PRIORITY 1: Check if clicked cell is a valid target cell
	if (ValidTargetCells.Contains(ClickedCell))
	{
		UE_LOG(LogTemp, Log, TEXT("Clicked cell [%d,%d] is a valid target"), ClickedRow, ClickedCol);

		// Get current ability and its targeting
		UUnitAbilityInstance* CurrentAbility = SelectedUnit->AbilityInventory ?
			SelectedUnit->AbilityInventory->GetCurrentActiveAbility() : nullptr;

		if (!CurrentAbility)
		{
			UE_LOG(LogTemp, Warning, TEXT("No active ability for selected unit"));
			return;
		}

		ETargetReach Reach = CurrentAbility->GetTargeting();

		// Get weapon's area shape if this is an Area ability
		const FAreaShape* AreaShape = nullptr;
		FAreaShape LocalAreaShape;
		if (Reach == ETargetReach::Area)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				UDamageCalculator* DamageCalc = World->GetSubsystem<UDamageCalculator>();
				if (DamageCalc)
				{
					UWeapon* Weapon = DamageCalc->SelectMaxReachWeapon(SelectedUnit);
					if (Weapon)
					{
						LocalAreaShape = Weapon->GetStats().AreaShape;
						AreaShape = &LocalAreaShape;
					}
				}
			}
		}

		// Resolve all targets based on clicked cell and reach type
		TArray<AUnit*> Targets = TargetingComponent->ResolveTargetsFromClick(
			SelectedUnit, ClickedCell, ClickedLayer, Reach, AreaShape);

		if (Targets.Num() > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Executing ability on %d target(s)"), Targets.Num());
			AbilityTargetSelected(SelectedUnit, Targets);
			return;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Valid target cell but no valid targets resolved at [%d,%d] Layer=%d"),
				ClickedRow, ClickedCol, (int32)ClickedLayer);
		}
	}

	// PRIORITY 2: Check if clicked cell is a valid move cell
	if (ValidMoveCells.Contains(ClickedCell))
	{
		UE_LOG(LogTemp, Log, TEXT("Clicked cell [%d,%d] is a valid move destination"), ClickedRow, ClickedCol);

		if (MoveUnit(SelectedUnit, ClickedRow, ClickedCol))
		{
			UE_LOG(LogTemp, Log, TEXT("Movement successful, ending turn"));
			ClearSelection();
			TurnManager->EndCurrentUnitTurn();
			return;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Movement failed for cell [%d,%d]"), ClickedRow, ClickedCol);
		}
	}

	// FALLBACK: Invalid click
	UE_LOG(LogTemp, Log, TEXT("Clicked cell [%d,%d] is neither a valid target nor a valid move"), ClickedRow, ClickedCol);
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

	// Build context via executor
	FAbilityBattleContext Context = AbilityExecutor->BuildContext(SourceUnit, Targets);

	// Execute ability via executor
	FAbilityResult Result = AbilityExecutor->ExecuteAbility(CurrentAbility, Context);

	// Resolve result
	AbilityExecutor->ResolveResult(Result);

	// Handle turn flow based on result
	if (Result.bSuccess)
	{
		switch (Result.TurnAction)
		{
		case EAbilityTurnAction::EndTurn:
			TurnManager->EndCurrentUnitTurn();
			break;

		case EAbilityTurnAction::FreeTurn:
			// Unit keeps turn - refresh selection
			SelectUnit(SourceUnit);
			break;

		case EAbilityTurnAction::EndTurnDelayed:
		case EAbilityTurnAction::RequireConfirm:
			// For now, just end turn (full implementation later)
			TurnManager->EndCurrentUnitTurn();
			break;
		}
	}
}

void ATacBattleGrid::HandleAbilityEquipped(UUnitAbilityInstance* Ability)
{
	if (!SelectedUnit || !Ability)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HandleAbilityEquipped: Refreshing targeting for new ability"));

	// Clear current highlights
	HighlightComponent->ClearHighlights();

	// Query new ability's targeting and refresh highlights
	const TArray<FIntPoint> TargetCells = GetValidTargetCells(SelectedUnit);
	HighlightComponent->ShowValidTargets(TargetCells);

	// Also refresh movement highlights
	const TArray<FIntPoint> ValidCells = MovementComponent->GetValidMoveCells(SelectedUnit);
	HighlightComponent->ShowValidMoves(ValidCells);
}

bool ATacBattleGrid::IsUnitOnFlank(const AUnit* Unit) const
{
	return DataManager->IsUnitOnFlank(Unit);
}

void ATacBattleGrid::SetUnitOnFlank(AUnit* Unit, bool bOnFlank)
{
	DataManager->SetUnitFlankState(Unit, bOnFlank);
}

FRotator ATacBattleGrid::GetUnitOriginalRotation(const AUnit* Unit) const
{
	return DataManager->GetUnitOriginalRotation(Unit);
}

void ATacBattleGrid::SetUnitOriginalRotation(AUnit* Unit, const FRotator& Rotation)
{
	DataManager->SetUnitOriginalRotation(Unit, Rotation);
}

TArray<AUnit*> ATacBattleGrid::GetTeamUnits(bool bIsAttackerTeam) const
{
	if (bIsAttackerTeam)
	{
		return AttackerTeam->GetUnits(); // Adjust based on UBattleTeam's API
	}
	else
	{
		return DefenderTeam->GetUnits();
	}
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

void ATacBattleGrid::InitializeComponents()
{
	DataManager->Initialize(this, AttackerTeam, DefenderTeam);
	MovementComponent->Initialize(this, DataManager);
	TargetingComponent->Initialize(this, DataManager);

	// Initialize presentation tracker and pass to dependent components
	TurnManager->PresentationTracker = PresentationTracker;

	AbilityExecutor->Initialize(this);
	AIController->Initialize(this, DataManager, MovementComponent, TargetingComponent, AbilityExecutor);
	HighlightComponent->Initialize(this, Root, Config->MoveAllowedDecalMaterial, Config->EnemyDecalMaterial);
	HighlightComponent->CreateDecalPool();

	// Subscribe to flank state changes
	MovementComponent->OnUnitFlankStateChanged.AddLambda(
		[this](AUnit* Unit, bool bEntering, FIntPoint Cell)
		{
			if (bEntering)
			{
				UnitEntersFlank(Unit, Cell.Y, Cell.X);
			}
			else
			{
				UnitExitsFlank(Unit);
			}
		}
	);
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
				UBattleTeam* Team = (Placement.bIsAttacker) ? AttackerTeam : DefenderTeam;
				Team->AddUnit(NewUnit);

				// Set unit's team side for movement orientation
				NewUnit->SetTeamSide(Team->GetTeamSide());

				if (!bIsFlank)
				{
					const float Yaw = (Team == AttackerTeam) ? 0.0f : 180.0f;
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

	TurnManager->AttackerTeam = AttackerTeam;
	TurnManager->DefenderTeam = DefenderTeam;

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
	AllBattleUnits.Append(AttackerTeam->GetUnits());
	AllBattleUnits.Append(DefenderTeam->GetUnits());

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

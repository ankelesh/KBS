#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "Components/DecalComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/Components/GridVisualMeshComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridHighlightComponent.h"
#include "GameMechanics/Tactical/Grid/Components/TacGridInputRouter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/CombatTypes.h"

#if WITH_EDITOR
#include "GameMechanics/Tactical/Grid/Editor/TacGridEditorInitializer.h"
#include "GameMechanics/Tactical/Grid/Editor/GridEditorVisualComponent.h"
#endif

ATacBattleGrid::ATacBattleGrid()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetMobility(EComponentMobility::Movable);
	VisualMesh = CreateDefaultSubobject<UGridVisualMeshComponent>(TEXT("VisualMesh"));

	DataManager = CreateDefaultSubobject<UGridDataManager>(TEXT("DataManager"));
	HighlightComponent = CreateDefaultSubobject<UGridHighlightComponent>(TEXT("HighlightComponent"));
	InputRouter = CreateDefaultSubobject<UTacGridInputRouter>(TEXT("InputRouter"));

#if WITH_EDITOR
	EditorInitializer = CreateDefaultSubobject<UTacGridEditorInitializer>(TEXT("EditorInitializer"));
	EditorVisuals = CreateDefaultSubobject<UGridEditorVisualComponent>(TEXT("EditorVisuals"));
#endif

	SetActorEnableCollision(true);
}
void ATacBattleGrid::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
#if WITH_EDITOR
	if (!bShowPreviewGizmos || !GetWorld() || GetWorld()->WorldType != EWorldType::Editor)
	{
		return;
	}
	FlushPersistentDebugLines(GetWorld());
	if (EditorVisuals)
	{
		EditorVisuals->DrawPreview();
	}
#endif
}
void ATacBattleGrid::BeginPlay()
{
	Super::BeginPlay();
	Player1ControlledTeam = Config->PlayerTeamSide;
	InitializeComponents();
	
	// Register with TacGridSubsystem
	UWorld* World = GetWorld();
	checkf(World, TEXT("TacBattleGrid: World is null in BeginPlay"));

	UTacGridSubsystem* GridSubsystem = World->GetSubsystem<UTacGridSubsystem>();
	checkf(GridSubsystem, TEXT("TacBattleGrid: Failed to get TacGridSubsystem"));

	// TODO: Move to subsystems
#if WITH_EDITOR
	if (EditorInitializer)
	{
		EditorInitializer->SpawnAndPlaceUnits();
		EditorInitializer->SetupUnitEventBindings();
	}
#endif

	// RegisterManager triggers the global init sequence - units must be on grid first
	GridSubsystem->RegisterManager(DataManager);
	UE_LOG(LogTemp, Log, TEXT("TacBattleGrid: Registered with TacGridSubsystem"));

}
void ATacBattleGrid::HandleUnitClicked(AUnit* Unit, FKey ButtonPressed)
{
	if (!Unit)
	{
		return;
	}
}



void ATacBattleGrid::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	if (InputRouter)
	{
		InputRouter->HandleGridClick(ButtonPressed);
	}
}

void ATacBattleGrid::InitializeComponents()
{
	DataManager->Initialize(this);
	HighlightComponent->Initialize(Root, Config);
	HighlightComponent->CreateHighlightPool();
	VisualMesh->Initialize(this);

	// Initialize InputRouter
	if (InputRouter)
	{
		InputRouter->Initialize(this, DataManager);
		UE_LOG(LogTemp, Log, TEXT("TacBattleGrid: InputRouter initialized"));
	}
}

bool ATacBattleGrid::GetCellUnderMouse(int32& OutRow, int32& OutCol, ETacGridLayer& OutLayer) const
{
	// TODO: Fix missing methods - CellFromWorldLocation, GetCellWorldLocation
	// APlayerController* PC = GetWorld()->GetFirstPlayerController();
	// if (!PC)
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("GetCellUnderMouse: No PlayerController found!"));
	// 	return false;
	// }
	// FVector WorldLocation, WorldDirection;
	// PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
	// FHitResult Hit;
	// FVector Start = WorldLocation;
	// FVector End = Start + WorldDirection * 10000.0f;
	// if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	// {
	// 	FIntPoint CellCoords = FTacCoordinates::WorldLocationToCell(Hit.Location, GetActorLocation(), Config->CellSize);
	// 	OutRow = CellCoords.Y;
	// 	OutCol = CellCoords.X;
	// 	FVector GroundLocation = FTacCoordinates::CellToWorldLocation(OutRow, OutCol, ETacGridLayer::Ground, GetActorLocation(), Config->CellSize, Config->AirLayerHeight);
	// 	FVector AirLocation = FTacCoordinates::CellToWorldLocation(OutRow, OutCol, ETacGridLayer::Air, GetActorLocation(), Config->CellSize, Config->AirLayerHeight);
	// 	float DistToGround = FMath::Abs(Hit.Location.Z - GroundLocation.Z);
	// 	float DistToAir = FMath::Abs(Hit.Location.Z - AirLocation.Z);
	// 	OutLayer = (DistToGround < DistToAir) ? ETacGridLayer::Ground : ETacGridLayer::Air;
	// 	return true;
	// }
	return false;
}

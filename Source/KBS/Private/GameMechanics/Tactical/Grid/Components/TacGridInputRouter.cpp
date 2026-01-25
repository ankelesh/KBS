#include "GameMechanics/Tactical/Grid/Components/TacGridInputRouter.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameplayTypes/AbilityTypes.h"
#include "Kismet/GameplayStatics.h"


UTacGridInputRouter::UTacGridInputRouter()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTacGridInputRouter::Initialize(ATacBattleGrid* InGrid, UGridDataManager* InDataManager)
{
	Grid = InGrid;
	DataManager = InDataManager;
}
void UTacGridInputRouter::HandleGridClick(FKey ButtonPressed)
{
	if (!Grid)
	{
		return;
	}

	FTacCoordinates ClickedCell;
	if (!GetCellUnderMouse(ClickedCell))
	{
		UE_LOG(LogTemp, Error, TEXT("InputRouter: Could not determine clicked cell"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("InputRouter: Grid clicked at cell [%d,%d] Layer=%d"), ClickedCell.Row, ClickedCell.Col, (int32)ClickedCell.Layer);

	// Forward to TurnSystem - let it handle all game logic
	UWorld* World = Grid->GetWorld();
	if (!World) return;

	// TODO: Forward to TacticalTurnSystem
	// UTacTurnSubsystem* TurnSystem = World->GetSubsystem<UTacTurnSubsystem>();
	// if (TurnSystem)
	// {
	// 	TurnSystem->HandleGridCellClicked(ClickedCell);
	// }
}
bool UTacGridInputRouter::GetCellUnderMouse(FTacCoordinates& OutCell) const
{
	if (!Grid)
	{
		return false;
	}
	UWorld* World = Grid->GetWorld();
	if (!World)
	{
		return false;
	}
	APlayerController* PC = World->GetFirstPlayerController();
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
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		OutCell = FTacCoordinates::WorldLocationToCell(Hit.Location, Grid->GetActorLocation(),
			Grid->GetCellSize(), Grid->GetAirLayerHeight());
		return true;
	}
	return false;
}

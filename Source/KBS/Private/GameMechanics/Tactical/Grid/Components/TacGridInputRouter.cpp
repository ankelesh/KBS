#include "GameMechanics/Tactical/Grid/Components/TacGridInputRouter.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
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
	checkf(InGrid, TEXT("InputRouter::Initialize: Grid cannot be null"));
	checkf(InDataManager, TEXT("InputRouter::Initialize: DataManager cannot be null"));

	Grid = InGrid;
	DataManager = InDataManager;
}
void UTacGridInputRouter::HandleGridClick(FKey ButtonPressed)
{
	check(Grid);

	FTacCoordinates ClickedCell;
	if (!GetCellUnderMouse(ClickedCell))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("InputRouter: Grid clicked at cell [%d,%d] Layer=%d"), ClickedCell.Row, ClickedCell.Col, (int32)ClickedCell.Layer);

	UWorld* World = Grid->GetWorld();
	check(World);

	UTacTurnSubsystem* TurnSystem = World->GetSubsystem<UTacTurnSubsystem>();
	if (TurnSystem)
	{
		TurnSystem->CellClicked(ClickedCell);
	}
}
bool UTacGridInputRouter::GetCellUnderMouse(FTacCoordinates& OutCell) const
{
	check(Grid);

	UWorld* World = Grid->GetWorld();
	check(World);

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
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

#include "GameMechanics/Tactical/Grid/Components/TacGridInputRouter.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridTargetingComponent.h"
#include "GameMechanics/Tactical/Grid/Components/TurnManagerComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridInputLockComponent.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Tactical/DamageCalculator.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameplayTypes/GridCoordinates.h"
#include "Kismet/GameplayStatics.h"
UTacGridInputRouter::UTacGridInputRouter()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UTacGridInputRouter::Initialize(
	ATacBattleGrid* InGrid,
	UGridDataManager* InDataManager,
	UGridMovementComponent* InMovementComponent,
	UGridTargetingComponent* InTargetingComponent,
	UTurnManagerComponent* InTurnManager,
	UGridInputLockComponent* InInputLockComponent)
{
	Grid = InGrid;
	DataManager = InDataManager;
	MovementComponent = InMovementComponent;
	TargetingComponent = InTargetingComponent;
	TurnManager = InTurnManager;
	InputLockComponent = InInputLockComponent;
}
void UTacGridInputRouter::HandleGridClick(FKey ButtonPressed)
{
	if (!TurnManager || !Grid)
	{
		return;
	}
	if (InputLockComponent && InputLockComponent->IsLocked())
	{
		UE_LOG(LogTemp, Log, TEXT("InputRouter: Input locked (%s), ignoring click"),
			*InputLockComponent->GetLockDebugInfo());
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("InputRouter: HandleGridClick called!"));
	AUnit* ActiveUnit = TurnManager->GetActiveUnit();
	if (!ActiveUnit)
	{
		UE_LOG(LogTemp, Log, TEXT("InputRouter: Grid clicked but no active unit"));
		return;
	}
	int32 ClickedRow, ClickedCol;
	EBattleLayer ClickedLayer;
	if (!GetCellUnderMouse(ClickedRow, ClickedCol, ClickedLayer))
	{
		UE_LOG(LogTemp, Error, TEXT("InputRouter: Could not determine clicked cell"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("InputRouter: Grid clicked at cell [%d,%d] Layer=%d"), ClickedRow, ClickedCol, (int32)ClickedLayer);
	const TArray<FIntPoint> ValidTargetCells = TargetingComponent->GetValidTargetCells(ActiveUnit);
	const TArray<FIntPoint> ValidMoveCells = MovementComponent->GetValidMoveCells(ActiveUnit);
	FIntPoint ClickedCell(ClickedCol, ClickedRow);
	if (ValidTargetCells.Contains(ClickedCell))
	{
		UE_LOG(LogTemp, Log, TEXT("InputRouter: Clicked cell [%d,%d] is a valid target"), ClickedRow, ClickedCol);
		UUnitAbilityInstance* CurrentAbility = ActiveUnit->AbilityInventory ?
			ActiveUnit->AbilityInventory->GetCurrentActiveAbility() : nullptr;
		if (!CurrentAbility)
		{
			UE_LOG(LogTemp, Warning, TEXT("InputRouter: No active ability for active unit"));
			return;
		}
		ETargetReach Reach = CurrentAbility->GetTargeting();
		const FAreaShape* AreaShape = nullptr;
		FAreaShape LocalAreaShape;
		if (Reach == ETargetReach::Area)
		{
			UWorld* World = Grid->GetWorld();
			if (World)
			{
				UDamageCalculator* DamageCalc = World->GetSubsystem<UDamageCalculator>();
				if (DamageCalc)
				{
					UWeapon* Weapon = DamageCalc->SelectMaxReachWeapon(ActiveUnit);
					if (Weapon)
					{
						LocalAreaShape = Weapon->GetStats().AreaShape;
						AreaShape = &LocalAreaShape;
					}
				}
			}
		}
		TArray<AUnit*> Targets = TargetingComponent->ResolveTargetsFromClick(
			ActiveUnit, ClickedCell, ClickedLayer, Reach, AreaShape);
		if (Targets.Num() > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("InputRouter: Executing ability on %d target(s)"), Targets.Num());
			TurnManager->ExecuteAbilityOnTargets(ActiveUnit, Targets);
			return;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("InputRouter: Valid target cell but no valid targets resolved at [%d,%d] Layer=%d"),
				ClickedRow, ClickedCol, (int32)ClickedLayer);
		}
	}
	if (ValidMoveCells.Contains(ClickedCell))
	{
		UE_LOG(LogTemp, Log, TEXT("InputRouter: Clicked cell [%d,%d] is a valid move destination"), ClickedRow, ClickedCol);
		if (MovementComponent->MoveUnit(ActiveUnit, ClickedRow, ClickedCol))
		{
			UE_LOG(LogTemp, Log, TEXT("InputRouter: Movement successful, ending turn"));
			TurnManager->EndCurrentUnitTurn();
			return;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("InputRouter: Movement failed for cell [%d,%d]"), ClickedRow, ClickedCol);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("InputRouter: Clicked cell [%d,%d] is neither a valid target nor a valid move"), ClickedRow, ClickedCol);
}
bool UTacGridInputRouter::GetCellUnderMouse(int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const
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
		FIntPoint CellCoords = Grid->GetCellFromWorldLocation(Hit.Location);
		OutRow = CellCoords.Y;
		OutCol = CellCoords.X;
		FVector GroundLocation = Grid->GetCellWorldLocation(OutRow, OutCol, EBattleLayer::Ground);
		FVector AirLocation = Grid->GetCellWorldLocation(OutRow, OutCol, EBattleLayer::Air);
		float DistToGround = FMath::Abs(Hit.Location.Z - GroundLocation.Z);
		float DistToAir = FMath::Abs(Hit.Location.Z - AirLocation.Z);
		OutLayer = (DistToGround < DistToAir) ? EBattleLayer::Ground : EBattleLayer::Air;
		return true;
	}
	return false;
}

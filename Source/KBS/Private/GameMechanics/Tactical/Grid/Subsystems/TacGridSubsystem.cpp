#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"

DEFINE_LOG_CATEGORY(LogTacGrid);
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridHighlightComponent.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacSubsystemControl.h"

void UTacGridSubsystem::RegisterManager(UGridDataManager* InDataManager)
{
	if (!InDataManager)
	{
		UE_LOG(LogTacGrid, Error, TEXT("TacGridSubsystem: Cannot register null DataManager"));
		return;
	}

	ATacBattleGrid* Grid = InDataManager->GetGrid();
	if (!Grid)
	{
		UE_LOG(LogTacGrid, Error, TEXT("TacGridSubsystem: DataManager has no Grid reference"));
		return;
	}

	UGridHighlightComponent* Highlight = Grid->GetHighlightComponent();
	if (!Highlight)
	{
		UE_LOG(LogTacGrid, Error, TEXT("TacGridSubsystem: Grid has no HighlightComponent"));
		return;
	}

	DataManager = InDataManager;
	HighlightComponent = Highlight;

	GridMovementService = NewObject<UTacGridMovementService>(this);
	GridMovementService->Initialize(InDataManager);
	GridTargetingService = NewObject<UTacGridTargetingService>(this);
	GridTargetingService->Initialize(InDataManager);
	UE_LOG(LogTacGrid, Log, TEXT("RegisterManager: grid '%s' ready"), *Grid->GetName());
	UTacSubsystemControl* Control = GetWorld()->GetSubsystem<UTacSubsystemControl>();
	checkf(Control, TEXT("TacGridSubsystem: TacSubsystemControl is nullptr"));
	Control->NotifyGridReady();
}

TArray<AUnit*> UTacGridSubsystem::GetActiveUnits()
{
	if (!DataManager) return TArray<AUnit*>();
	return DataManager->GetAllAliveUnits();
}

TArray<AUnit*> UTacGridSubsystem::GetAllUnits()
{
	if (!DataManager) return TArray<AUnit*>();
	return DataManager->GetAllUnits();
}

TArray<AUnit*> UTacGridSubsystem::GetDeadUnits()
{
	if (!DataManager) return TArray<AUnit*>();
	return DataManager->GetAllDeadUnits();
}

UBattleTeam* UTacGridSubsystem::GetAttackerTeam()
{
	
	return DataManager->GetAttackerTeam();
}

UBattleTeam* UTacGridSubsystem::GetDefenderTeam()
{
	
	return DataManager->GetDefenderTeam();
}

UBattleTeam* UTacGridSubsystem::GetPlayerTeam()
{
	return DataManager->GetPlayerTeam();
}

bool UTacGridSubsystem::IsBothTeamsAnyUnitAlive()
{
	if (!DataManager) return false;
	return DataManager->IsBothTeamsAnyUnitAlive();
}

UBattleTeam* UTacGridSubsystem::GetWinnerTeam()
{
	

	UBattleTeam* AttackerTeam = DataManager->GetAttackerTeam();
	UBattleTeam* DefenderTeam = DataManager->GetDefenderTeam();

	if (!AttackerTeam || !DefenderTeam) return nullptr;

	bool bAttackerAlive = AttackerTeam->IsAnyUnitAlive();
	bool bDefenderAlive = DefenderTeam->IsAnyUnitAlive();

	if (bAttackerAlive && !bDefenderAlive)
	{
		return AttackerTeam;
	}
	if (bDefenderAlive && !bAttackerAlive)
	{
		return DefenderTeam;
	}

	return nullptr;
}

void UTacGridSubsystem::ShowHighlights(const TArray<FTacCoordinates>& Cells, EHighlightType HighlightType)
{
	HighlightComponent->ShowHighlights(Cells, HighlightType);
}

void UTacGridSubsystem::ClearHighlights(EHighlightType HighlightType)
{
	HighlightComponent->ClearHighlights(HighlightType);
}

void UTacGridSubsystem::ClearAllHighlights()
{
	HighlightComponent->ClearAllHighlights();
}

bool UTacGridSubsystem::GetUnitCoordinates(const AUnit* Unit, FTacCoordinates& OutCoordinates) const
{
	if (!DataManager || !Unit)
	{
		return false;
	}

	ETacGridLayer OutLayer;
	return DataManager->GetUnitPosition(Unit, OutCoordinates, OutLayer);
}

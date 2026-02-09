#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridHighlightComponent.h"


void UTacGridSubsystem::RegisterManager(UGridDataManager* InDataManager)
{
	if (!InDataManager)
	{
		UE_LOG(LogTemp, Error, TEXT("TacGridSubsystem: Cannot register null DataManager"));
		return;
	}

	ATacBattleGrid* Grid = InDataManager->GetGrid();
	if (!Grid)
	{
		UE_LOG(LogTemp, Error, TEXT("TacGridSubsystem: DataManager has no Grid reference"));
		return;
	}

	UGridHighlightComponent* Highlight = Grid->GetHighlightComponent();
	if (!Highlight)
	{
		UE_LOG(LogTemp, Error, TEXT("TacGridSubsystem: Grid has no HighlightComponent"));
		return;
	}

	DataManager = InDataManager;
	HighlightComponent = Highlight;

	GridMovementService = NewObject<UTacGridMovementService>(this);
	GridMovementService->Initialize(InDataManager);
	GridTargetingService = NewObject<UTacGridTargetingService>(this);
	GridTargetingService->Initialize(InDataManager);

	UE_LOG(LogTemp, Log, TEXT("TacGridSubsystem: Registered successfully with Grid and HighlightComponent"));
}

TArray<TObjectPtr<AUnit>> UTacGridSubsystem::GetActiveUnits()
{
	if (!DataManager) return TArray<TObjectPtr<AUnit>>();
	return DataManager->GetAllAliveUnits();
}

TArray<TObjectPtr<AUnit>> UTacGridSubsystem::GetAllUnits()
{
	if (!DataManager) return TArray<TObjectPtr<AUnit>>();
	return DataManager->GetAllUnits();
}

TArray<TObjectPtr<AUnit>> UTacGridSubsystem::GetDeadUnits()
{
	if (!DataManager) return TArray<TObjectPtr<AUnit>>();
	return DataManager->GetAllDeadUnits();
}

TObjectPtr<UBattleTeam> UTacGridSubsystem::GetAttackerTeam()
{
	if (!DataManager) return nullptr;
	return DataManager->GetAttackerTeam();
}

TObjectPtr<UBattleTeam> UTacGridSubsystem::GetDefenderTeam()
{
	if (!DataManager) return nullptr;
	return DataManager->GetDefenderTeam();
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

#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"

DEFINE_LOG_CATEGORY(LogTacGrid);
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "GameMechanics/Units/UnitDefinition.h"
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

	for (AUnit* Unit : DataManager->GetUnits(EUnitQuerySource::OnField))
	{
		Unit->OnUnitDied.AddDynamic(this, &UTacGridSubsystem::HandleUnitDied);
	}

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
	return DataManager->GetUnits(EUnitQuerySource::OnField);
}

TArray<AUnit*> UTacGridSubsystem::GetAllAliveUnits()
{
	if (!DataManager) return TArray<AUnit*>();
	return DataManager->GetUnits(EUnitQuerySource::OnField | EUnitQuerySource::OffField);
}

TArray<AUnit*> UTacGridSubsystem::GetAllUnits()
{
	if (!DataManager) return TArray<AUnit*>();
	return DataManager->GetUnits(EUnitQuerySource::OnField | EUnitQuerySource::OffField | EUnitQuerySource::Corpses);
}

TArray<AUnit*> UTacGridSubsystem::GetDeadUnits()
{
	if (!DataManager) return TArray<AUnit*>();
	return DataManager->GetUnits(EUnitQuerySource::Corpses);
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
	return DataManager->GetAttackerTeam()->IsAnyUnitAlive() && DataManager->GetDefenderTeam()->IsAnyUnitAlive();
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

	if (!Unit->GridMetadata.IsOnField())
	{
		return false;
	}
	OutCoordinates = Unit->GridMetadata.Coords;
	return true;
}

TArray<AUnit*> UTacGridSubsystem::GetOffFieldUnits() const
{
	if (!DataManager) return TArray<AUnit*>();
	return DataManager->GetOffFieldUnits();
}

bool UTacGridSubsystem::IsUnitOffField(const AUnit* Unit) const
{
	if (!DataManager) return false;
	return DataManager->IsUnitOffField(Unit);
}

void UTacGridSubsystem::PlaceUnitOffField(AUnit* Unit)
{
	DataManager->PlaceUnitOffField(Unit);
}

void UTacGridSubsystem::HandleUnitDied(AUnit* Unit)
{
	if (!Unit->GridMetadata.IsOnField())
	{
		return;
	}
	DataManager->PushCorpse(Unit, Unit->GridMetadata.Coords);
	DataManager->RemoveUnit(Unit);
}

AUnit* UTacGridSubsystem::SpawnSummonedUnit(TSubclassOf<AUnit> UnitClass, UUnitDefinition* Definition,
                                             FTacCoordinates Cell, UBattleTeam* Team)
{
	AUnit* NewUnit = DataManager->SpawnUnit(UnitClass, Definition, Cell, Team);
	if (!NewUnit) return nullptr;

	NewUnit->OnUnitDied.AddDynamic(this, &UTacGridSubsystem::HandleUnitDied);

	UTacTurnSubsystem* TurnSub = GetWorld()->GetSubsystem<UTacTurnSubsystem>();
	checkf(TurnSub, TEXT("SpawnSummonedUnit: TurnSubsystem missing"));
	TurnSub->RegisterSummonedUnit(NewUnit);

	return NewUnit;
}

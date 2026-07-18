#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "TacGridSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTacGrid, Log, All);


class UTacGridMovementService;
class UTacGridTargetingService;
class AUnit;
class UBattleTeam;
class UUnitDefinition;
class UTacticalPresentationBuilderConfig;
enum class EHighlightType : uint8;
UCLASS()
class KBS_API UTacGridSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UTacGridSubsystem() {}
	void RegisterManager(UGridDataManager* InDataManager);

	UTacGridMovementService* GetGridMovementService() { return GridMovementService; }
	UTacGridTargetingService* GetGridTargetingService() { return GridTargetingService; }
	AUnit* SpawnSummonedUnit(TSubclassOf<AUnit> UnitClass, UUnitDefinition* Definition,
	                         FTacCoordinates Cell, UBattleTeam* Team);

	void ShowHighlights(const TArray<FTacCoordinates>& Cells, EHighlightType HighlightType);
	void ClearHighlights(EHighlightType HighlightType);
	void ClearAllHighlights();

	TArray<AUnit*> GetActiveUnits();       // on-field alive
	TArray<AUnit*> GetAllAliveUnits();     // on-field + off-field alive
	TArray<AUnit*> GetAllUnits();          // everything
	TArray<AUnit*> GetDeadUnits();
	UBattleTeam* GetAttackerTeam();
	UBattleTeam* GetDefenderTeam();
	UBattleTeam* GetPlayerTeam();
	bool IsBothTeamsAnyUnitAlive();
	UBattleTeam* GetWinnerTeam();

	bool GetUnitCoordinates(const AUnit* Unit, FTacCoordinates& OutCoordinates) const;
	FVector GetCellWorldLocation(FTacCoordinates Coords) const { return DataManager->GetCellWorldLocation(Coords); }

	TArray<AUnit*> GetOffFieldUnits() const;
	bool IsUnitOffField(const AUnit* Unit) const;
	void PlaceUnitOffField(AUnit* Unit);

	// Optional override asset assigned on the level's UGridConfig. Null if unset - callers fall back to defaults.
	UTacticalPresentationBuilderConfig* GetPresentationConfig() const;

	// Kills a unit outside combat (summon expiry, replacement, etc.) without leaving a corpse on the
	// field. Does NOT destroy the unit - it stays in a pending buffer until FinalizeDespawnedUnit is
	// called once presentation has played the despawn action.
	void DespawnUnit(AUnit* Unit);
	// Called by UDespawnPresentationAction::OnCleanup once the despawn's presentation has fully played out.
	void FinalizeDespawnedUnit(AUnit* Unit);

private:
	UFUNCTION()
	void HandleUnitDied(AUnit* Unit);


	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;
	UPROPERTY()
	TObjectPtr<UTacGridMovementService> GridMovementService;
	UPROPERTY()
	TObjectPtr<UTacGridTargetingService> GridTargetingService;
	UPROPERTY()
	TObjectPtr<class UGridHighlightComponent> HighlightComponent;




};
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTypes/GridCoordinates.h"
#include "TacGridSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTacGrid, Log, All);


class UGridDataManager;
class UTacGridMovementService;
class UTacGridTargetingService;
class AUnit;
class UBattleTeam;
class UUnitDefinition;
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

	TArray<AUnit*> GetOffFieldUnits() const;
	bool IsUnitOffField(const AUnit* Unit) const;

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
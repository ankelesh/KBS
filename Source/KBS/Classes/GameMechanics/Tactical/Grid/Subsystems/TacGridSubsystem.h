#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTypes/GridCoordinates.h"
#include "TacGridSubsystem.generated.h"


class UGridDataManager;
class UTacGridMovementService;
class UTacGridTargetingService;
class AUnit;
class UBattleTeam;
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

	void ShowHighlights(const TArray<FTacCoordinates>& Cells, EHighlightType HighlightType);
	void ClearHighlights(EHighlightType HighlightType);
	void ClearAllHighlights();

	TArray<AUnit*> GetActiveUnits();
	TArray<AUnit*> GetAllUnits();
	TArray<AUnit*> GetDeadUnits();
	UBattleTeam* GetAttackerTeam();
	UBattleTeam* GetDefenderTeam();
	UBattleTeam* GetPlayerTeam();
	bool IsBothTeamsAnyUnitAlive();
	UBattleTeam* GetWinnerTeam();

	bool GetUnitCoordinates(const AUnit* Unit, FTacCoordinates& OutCoordinates) const;

	
private:
	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;
	UPROPERTY()
	TObjectPtr<UTacGridMovementService> GridMovementService;
	UPROPERTY()
	TObjectPtr<UTacGridTargetingService> GridTargetingService;
	UPROPERTY()
	TObjectPtr<class UGridHighlightComponent> HighlightComponent;




};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridQueryPredicates.h"
#include "GameplayTypes/GridCoordinates.h"
#include "TacGridTargetingService.generated.h"

class AUnit;
class UGridDataManager;
class UCombatDescriptor;
enum class ETeamSide : uint8;

enum class ETargetReach : uint8;


UCLASS()
class KBS_API UTacGridTargetingService : public UObject
{
	GENERATED_BODY()

public:
	UTacGridTargetingService();
	void Initialize(UGridDataManager* InDataManager);
	
	
	TArray<FTacCoordinates> GetValidTargetCells(AUnit* Unit, ETargetReach Reach) const;
	TArray<AUnit*> GetValidTargetUnits(AUnit* Unit, ETargetReach Reach, bool bIncludeSelf = false) const;
	
	
	struct FResolvedTargets ResolveTargetsFromClick(AUnit* SourceUnit, FTacCoordinates ClickedCell, ETargetReach Reach,
	                                                const struct FAreaShape* AreaShape = nullptr) const;
	bool HasValidTargetAtCell(AUnit* Source, FTacCoordinates TargetCell, ETargetReach Reach) const;
	bool HasAnyValidTargets(AUnit* Source, ETargetReach Reach) const;

private:
	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;
	void GetClosestEnemyCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
	bool CanTargetClosestCell(AUnit* SourceUnit, EAffiliationFilter Filter, FTacCoordinates Cell) const;
	bool CanSelfTarget(AUnit* Source, FTacCoordinates Coord) const;
	void GetCellsByAffiliation(AUnit* SourceUnit, EAffiliationFilter Filter, TArray<FTacCoordinates>& OutCells,
	                           bool bAllowEmpty, bool bAllowFlank) const;
	void GetMovementCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
	bool TestMovementCell(AUnit* Unit, FTacCoordinates Cell) const;
	void GetCorpseCellsByAffiliation(AUnit* Unit, EAffiliationFilter Filter, bool bAllowBlocked,
	                                 TArray<FTacCoordinates>& OutCells) const;
	void GetGroundMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
	void GetMultiCellGroundMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
	void GetAirMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
	void GetCellsInArea(AUnit* Unit, FTacCoordinates CenterCell, EAffiliationFilter Filter, const FAreaShape& AreaShape,
	                    TArray<FTacCoordinates>& OutCells) const;
	bool IsAdjacent(AUnit* Source, const FTacCoordinates& Coords) const;
	bool CheckUnitAndData(AUnit* Unit) const;
	bool TestCell(AUnit* SourceUnit, FTacCoordinates Cell, QueryPredicates::FCellFilterPredicate Predicate) const;
	bool TestCell(FTacCoordinates Cell, QueryPredicates::FCorpseFilterPredicate Predicate) const;
	FResolvedTargets BuildTargetsFromCells(FTacCoordinates ClickedCell, TArray<FTacCoordinates>& QueriedCells) const;
	FResolvedTargets BuildTargetFromCell(FTacCoordinates ClickedCell, bool bHasPassedFilter) const;
};

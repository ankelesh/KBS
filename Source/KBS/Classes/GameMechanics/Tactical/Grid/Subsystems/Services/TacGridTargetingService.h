// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridQueryPredicates.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/TargetingDescriptor.h"
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
	
	
	TArray<FTacCoordinates> GetValidTargetCells(AUnit* Unit, FTargetingDescriptor Desc) const;
	TArray<AUnit*> GetValidTargetUnits(AUnit* Unit, FTargetingDescriptor Desc, bool bIncludeSelf = false) const;
	FResolvedTargets ResolveTargetsFromClick(AUnit* SourceUnit, FTacCoordinates ClickedCell,
	                                                FTargetingDescriptor Desc,
	                                                const struct FAreaShape* AreaShape = nullptr) const;
	bool HasValidTargetAtCell(AUnit* Source, FTacCoordinates TargetCell, FTargetingDescriptor Desc) const;
	bool HasAnyValidTargets(AUnit* Source, FTargetingDescriptor Desc) const;

	// Legacy overloads — thin wrappers kept for incremental migration
	TArray<FTacCoordinates> GetValidTargetCells(AUnit* Unit, ETargetReach Reach) const;
	TArray<AUnit*> GetValidTargetUnits(AUnit* Unit, ETargetReach Reach, bool bIncludeSelf = false) const;
	FResolvedTargets ResolveTargetsFromClick(AUnit* SourceUnit, FTacCoordinates ClickedCell, ETargetReach Reach,
	                                                const struct FAreaShape* AreaShape = nullptr) const;
	bool HasValidTargetAtCell(AUnit* Source, FTacCoordinates TargetCell, ETargetReach Reach) const;
	bool HasAnyValidTargets(AUnit* Source, ETargetReach Reach) const;

private:
	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;
	TArray<FTacCoordinates> GetClosestEnemyCells(AUnit* Unit, const FTargetingDescriptor& Desc) const;
	bool CanTargetClosestCell(AUnit* SourceUnit, const FTargetingDescriptor& Desc, FTacCoordinates Cell) const;
	bool CanSelfTarget(AUnit* Source, FTacCoordinates Coord) const;
	TArray<FTacCoordinates> GetCellsByAffiliation(AUnit* SourceUnit, const FTargetingDescriptor& Desc) const;
	bool TestMovementCell(AUnit* Unit, FTacCoordinates Cell, const FTargetingDescriptor& Desc) const;
	TArray<FTacCoordinates> GetCorpseCellsByAffiliation(AUnit* Unit, const FTargetingDescriptor& Desc) const;

	// Movement — master + size helpers
	TArray<FTacCoordinates> GetValidMovementCells (AUnit* Unit, const FTargetingDescriptor& Desc) const;
	TArray<FTacCoordinates> GetSingleCellMoveCells(AUnit* Unit, const FTargetingDescriptor& Desc) const;
	TArray<FTacCoordinates> GetMultiCellMoveCells (AUnit* Unit, const FTargetingDescriptor& Desc) const;

	// Movement — 6 leaf helpers (size × pattern), each resolves layer/predicate from Desc
	TArray<FTacCoordinates> GetSingleCellOrthogonal(AUnit* Unit, const FTargetingDescriptor& Desc) const;
	TArray<FTacCoordinates> GetSingleCellAnyToAny  (AUnit* Unit, const FTargetingDescriptor& Desc) const;
	TArray<FTacCoordinates> GetSingleCellLinear    (AUnit* Unit, const FTargetingDescriptor& Desc) const;
	TArray<FTacCoordinates> GetMultiCellOrthogonal (AUnit* Unit, const FTargetingDescriptor& Desc) const;
	TArray<FTacCoordinates> GetMultiCellAnyToAny   (AUnit* Unit, const FTargetingDescriptor& Desc) const;
	TArray<FTacCoordinates> GetMultiCellLinear     (AUnit* Unit, const FTargetingDescriptor& Desc) const;
	TArray<FTacCoordinates> GetCellsInArea(AUnit* Unit, FTacCoordinates CenterCell,
	                                       const FTargetingDescriptor& Desc, const FAreaShape& AreaShape) const;
	bool CheckUnitAndData(AUnit* Unit) const;
	bool TestCell(AUnit* SourceUnit, FTacCoordinates Cell, QueryPredicates::FCellFilterPredicate Predicate) const;
	bool TestCell(FTacCoordinates Cell, QueryPredicates::FCorpseFilterPredicate Predicate) const;
	FResolvedTargets BuildTargetsFromCells(FTacCoordinates ClickedCell, const TArray<FTacCoordinates>& QueriedCells) const;
	FResolvedTargets BuildTargetFromCell(FTacCoordinates ClickedCell, bool bHasPassedFilter) const;
};

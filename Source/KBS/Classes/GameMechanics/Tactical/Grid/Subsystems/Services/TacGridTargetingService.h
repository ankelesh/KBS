// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameMechanics/GameplayTypes/GridCoordinates.h"
#include "TacGridTargetingService.generated.h"

namespace Tactical {
	class AUnit;
	class UGridDataManager;

	enum class ECorpseFilter : uint8
	{
		Any,
		Friendly,
		Enemy,
		AnyNonBlocked,
		FriendlyNonBlocked,
		EnemyNonBlocked
	};

	UCLASS()
		class KBS_API UTacGridTargetingService : public UObject
	{
		GENERATED_BODY()
	public:
		UTacGridTargetingService();
		void Initialize(UGridDataManager* InDataManager);
		TArray<FTacCoordinates> GetValidTargetCells(AUnit* Unit, ETargetReach Reach, bool bUseFlankTargeting = false) const;
		TArray<AUnit*> GetValidTargetUnits(AUnit* Unit, ETargetReach Reach, bool bUseFlankTargeting = false, bool bIncludeDeadUnits = false) const;
		TArray<AUnit*> ResolveTargetsFromClick(AUnit* SourceUnit, FTacCoordinates ClickedCell, ETargetReach Reach, const struct FAreaShape* AreaShape = nullptr) const;
		bool IsValidTargetCell(AUnit* Unit, const FTacCoordinates& Cell, ETargetReach Reach) const;
	private:
		UPROPERTY()
		TObjectPtr<UGridDataManager> DataManager;
		void GetClosestEnemyCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetFlankTargetCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetAnyEnemyCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetAllFriendlyCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetEmptyCellsOrFriendly(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetMovementCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetCorpseCells(AUnit* Unit, ECorpseFilter Filter, TArray<FTacCoordinates>& OutCells) const;
	void GetAnyCorpseCells(TArray<FTacCoordinates>& OutCells) const;
		void GetFriendlyCorpseCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetEnemyCorpseCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetAnyNonBlockedCorpseCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetFriendlyNonBlockedCorpseCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetEnemyNonBlockedCorpseCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetAdjacentMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetFlankMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetAirMoveCells(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		bool CanEnterFlankCell(const FTacCoordinates& UnitPos, const FTacCoordinates& FlankCell, class UBattleTeam* UnitTeam) const;
		bool IsAdjacentCell(const FTacCoordinates& CellA, const FTacCoordinates& CellB) const;
		bool IsFlankCell(const FTacCoordinates& Cell) const;
		bool IsValidMultiCellDestination(AUnit* Unit, const FTacCoordinates& TargetCoords, EBattleLayer Layer) const;
		TArray<AUnit*> GetUnitsInArea(FTacCoordinates CenterCell, const struct FAreaShape& AreaShape) const;
		bool TryGetPrimaryCellForUnit(AUnit* Unit, FTacCoordinates& OutCell) const;
		void AddUnitCell(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void AddUnitCellUnique(AUnit* Unit, TArray<FTacCoordinates>& OutCells) const;
		void GetEmptyCellsForLayer(EBattleLayer Layer, TArray<FTacCoordinates>& OutCells) const;
	void IterateGroundCells(TFunctionRef<bool(const FTacCoordinates&)> FilterPredicate, TArray<FTacCoordinates>& OutCells) const;
	};
}; //namespace Tactical
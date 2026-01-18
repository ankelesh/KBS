#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameMechanics/GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Units/Unit.h"
#include "GridDataManager.generated.h"

namespace Tactical {
	class ATacBattleGrid;
	USTRUCT()
		struct FCorpseStack
	{
		GENERATED_BODY()
		UPROPERTY()
		TArray<TObjectPtr<AUnit>> Corpses;

		void Push(AUnit* Unit, const FVector& WorldLocation);
		AUnit* Pop();
		AUnit* Top() const;
		int32 Num() const { return Corpses.Num(); }
		bool IsEmpty() const { return Corpses.IsEmpty(); }
		const TArray<TObjectPtr<AUnit>>& GetAll() const { return Corpses; }

	private:
		void SetCorpseVisibility(AUnit* Corpse, bool bVisible);
	};
	USTRUCT()
		struct FGridRow
	{
		GENERATED_BODY()
		UPROPERTY(EditAnywhere)
		TArray<TObjectPtr<AUnit>> Cells;
		UPROPERTY(EditAnywhere)
		TArray<FCorpseStack> CorpseStacks;
		FGridRow()
		{
			Cells.Init(nullptr, 5);
			CorpseStacks.SetNum(5);
		}
	};



	USTRUCT()
		struct FMultiCellUnitData
	{
		GENERATED_BODY()
		UPROPERTY()
		TArray<FTacCoordinates> OccupiedCells;
		UPROPERTY()
		FTacCoordinates PrimaryCell;
		UPROPERTY()
		bool bIsHorizontal = false;
	};
	UCLASS()
		class KBS_API UGridDataManager : public UObject
	{
		GENERATED_BODY()
	public:
		void Initialize(ATacBattleGrid* InGrid);

		// Primary FTacCoordinates-based interface
		bool PlaceUnit(AUnit* Unit, FTacCoordinates Coords);
		AUnit* GetUnit(FTacCoordinates Coords) const;
		bool RemoveUnit(FTacCoordinates Coords);

		// Convenience overloads (delegate to FTacCoordinates versions)
		bool PlaceUnit(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer, ATacBattleGrid* TacBattleGrid);
		AUnit* GetUnit(int32 Row, int32 Col, EBattleLayer Layer) const;
		bool RemoveUnit(int32 Row, int32 Col, EBattleLayer Layer, ATacBattleGrid* TacBattleGrid);
		TArray<FGridRow>& GetLayer(EBattleLayer Layer);
		const TArray<FGridRow>& GetLayer(EBattleLayer Layer) const;
		bool GetUnitPosition(const AUnit* Unit, FTacCoordinates& OutPosition, EBattleLayer& OutLayer) const;
		bool IsUnitOnFlank(const AUnit* Unit) const;
		void SetUnitFlankState(AUnit* Unit, bool bOnFlank);
		FRotator GetUnitOriginalRotation(const AUnit* Unit) const;
		void SetUnitOriginalRotation(AUnit* Unit, const FRotator& Rotation);
		bool IsValidCell(FTacCoordinates Coords) const;
		bool IsFlankCell(FTacCoordinates Coords) const;
		bool IsRestrictedCell(FTacCoordinates Coords) const;
		FVector GetCellWorldLocation(FTacCoordinates Coords) const;
		UBattleTeam* GetTeamForUnit(AUnit* Unit) const;
		UBattleTeam* GetEnemyTeam(AUnit* Unit) const;
		UBattleTeam* GetAttackerTeam() const { return AttackerTeam; }
		UBattleTeam* GetDefenderTeam() const { return DefenderTeam; }
		ATacBattleGrid* GetGrid() const { return Grid; }
		UFUNCTION(BlueprintCallable, Category = "Grid|Teams")
		TArray<AUnit*> GetUnitsFromTeam(bool bIsAttacker) const;
		TArray<FIntPoint> GetEmptyCells(EBattleLayer Layer) const;
		TArray<FIntPoint> GetOccupiedCells(EBattleLayer Layer, UBattleTeam* Team) const;
		bool IsCellOccupied(FTacCoordinates Coords) const;
		TArray<FIntPoint> GetValidPlacementCells(EBattleLayer Layer) const;
		void PushCorpse(AUnit* Unit, FTacCoordinates Coords);
		AUnit* GetTopCorpse(FTacCoordinates Coords) const;
		AUnit* PopCorpse(FTacCoordinates Coords);
		bool HasCorpses(FTacCoordinates Coords) const;
		const TArray<TObjectPtr<AUnit>>& GetCorpseStack(FTacCoordinates Coords) const;
		TArray<AUnit*> GetUnitsInCells(const TArray<FTacCoordinates>& CellCoords, EBattleLayer Layer) const;
		bool IsMultiCellUnit(const AUnit* Unit) const;
		const FMultiCellUnitData* GetMultiCellData(const AUnit* Unit) const;
	private:
		UPROPERTY()
		TArray<FGridRow> GroundLayer;
		UPROPERTY()
		TArray<FGridRow> AirLayer;
		TMap<AUnit*, bool> UnitFlankStates;
		TMap<AUnit*, FRotator> UnitOriginalRotations;
		UPROPERTY()
		TMap<TObjectPtr<AUnit>, FMultiCellUnitData> MultiCellUnits;
		UPROPERTY()
		TObjectPtr<ATacBattleGrid> Grid;
		UPROPERTY()
		TObjectPtr<UBattleTeam> AttackerTeam;
		UPROPERTY()
		TObjectPtr<UBattleTeam> DefenderTeam;
		FVector GridWorldLocation;
	};
}; // namespace Tactical
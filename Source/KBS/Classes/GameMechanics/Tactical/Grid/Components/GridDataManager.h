#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameMechanics/Units/Unit.h"
#include "GridDataManager.generated.h"
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
struct FGridCellCoord
{
	GENERATED_BODY()
	UPROPERTY()
	int32 Row = 0;
	UPROPERTY()
	int32 Col = 0;
	UPROPERTY()
	EBattleLayer Layer = EBattleLayer::Ground;

	FGridCellCoord() = default;
	FGridCellCoord(int32 InRow, int32 InCol, EBattleLayer InLayer)
		: Row(InRow), Col(InCol), Layer(InLayer) {}

	bool operator==(const FGridCellCoord& Other) const
	{
		return Row == Other.Row && Col == Other.Col && Layer == Other.Layer;
	}
};

USTRUCT()
struct FMultiCellUnitData
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<FGridCellCoord> OccupiedCells;
	UPROPERTY()
	FGridCellCoord PrimaryCell;
	UPROPERTY()
	bool bIsHorizontal = false;
};
UCLASS()
class KBS_API UGridDataManager : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(ATacBattleGrid* InGrid);
	bool PlaceUnit(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer, ATacBattleGrid* Grid);
	AUnit* GetUnit(int32 Row, int32 Col, EBattleLayer Layer) const;
	bool RemoveUnit(int32 Row, int32 Col, EBattleLayer Layer, ATacBattleGrid* Grid);
	TArray<FGridRow>& GetLayer(EBattleLayer Layer);
	const TArray<FGridRow>& GetLayer(EBattleLayer Layer) const;
	bool GetUnitPosition(const AUnit* Unit, int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const;
	bool IsUnitOnFlank(const AUnit* Unit) const;
	void SetUnitFlankState(AUnit* Unit, bool bOnFlank);
	FRotator GetUnitOriginalRotation(const AUnit* Unit) const;
	void SetUnitOriginalRotation(AUnit* Unit, const FRotator& Rotation);
	bool IsValidCell(int32 Row, int32 Col, EBattleLayer Layer) const;
	bool IsFlankCell(int32 Row, int32 Col) const;
	bool IsRestrictedCell(int32 Row, int32 Col) const;
	FVector GetCellWorldLocation(int32 Row, int32 Col, EBattleLayer Layer) const;
	UBattleTeam* GetTeamForUnit(AUnit* Unit) const;
	UBattleTeam* GetEnemyTeam(AUnit* Unit) const;
	UBattleTeam* GetAttackerTeam() const { return AttackerTeam; }
	UBattleTeam* GetDefenderTeam() const { return DefenderTeam; }
	UFUNCTION(BlueprintCallable, Category = "Grid|Teams")
	TArray<AUnit*> GetUnitsFromTeam(bool bIsAttacker) const;
	TArray<FIntPoint> GetEmptyCells(EBattleLayer Layer) const;
	TArray<FIntPoint> GetOccupiedCells(EBattleLayer Layer, UBattleTeam* Team) const;
	bool IsCellOccupied(int32 Row, int32 Col, EBattleLayer Layer) const;
	TArray<FIntPoint> GetValidPlacementCells(EBattleLayer Layer) const;
	void PushCorpse(AUnit* Unit, int32 Row, int32 Col);
	AUnit* GetTopCorpse(int32 Row, int32 Col) const;
	AUnit* PopCorpse(int32 Row, int32 Col);
	bool HasCorpses(int32 Row, int32 Col) const;
	const TArray<TObjectPtr<AUnit>>& GetCorpseStack(int32 Row, int32 Col) const;
	TArray<AUnit*> GetUnitsInCells(const TArray<FIntPoint>& CellCoords, EBattleLayer Layer) const;
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

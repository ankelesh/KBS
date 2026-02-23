#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Units/Unit.h"
#include "GridDataManager.generated.h"

class ATacBattleGrid;
class UUnitDefinition;
#if WITH_EDITOR
class UTacGridEditorInitializer;
#endif

// Flags selecting which unit storage locations to include in a query.
// OnField cells contain alive units by contract; Corpses contain dead units.
enum class EUnitQuerySource : uint8
{
	OnField  = 0x01,
	OffField = 0x02,
	Corpses  = 0x04,
};
ENUM_CLASS_FLAGS(EUnitQuerySource)

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
		Cells.Init(nullptr, FGridConstants::GridSize);
		CorpseStacks.SetNum(FGridConstants::GridSize);
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
	bool PlaceUnit(AUnit* Unit, int32 Row, int32 Col, ETacGridLayer Layer);
	AUnit* GetUnit(int32 Row, int32 Col, ETacGridLayer Layer) const;
	bool RemoveUnit(int32 Row, int32 Col, ETacGridLayer Layer);

	bool GetUnitPosition(const AUnit* Unit, FTacCoordinates& OutPosition, ETacGridLayer& OutLayer) const;
	bool IsUnitOnFlank(const AUnit* Unit) const;
	void SetUnitFlankState(AUnit* Unit, bool bOnFlank);
	FRotator GetUnitOriginalRotation(const AUnit* Unit) const;
	void SetUnitOriginalRotation(AUnit* Unit, const FRotator& Rotation);
	FVector GetCellWorldLocation(FTacCoordinates Coords) const;

	UBattleTeam* GetTeamBySide(ETeamSide Side) const;
	UBattleTeam* GetAttackerTeam() const { return AttackerTeam; }
	UBattleTeam* GetDefenderTeam() const { return DefenderTeam; }
	UBattleTeam* GetPlayerTeam() const { return bPlayerIsAttacker ? AttackerTeam : DefenderTeam; }
	ATacBattleGrid* GetGrid() const { return Grid; }

	TArray<FTacCoordinates> GetEmptyCells(ETacGridLayer Layer) const;
	TArray<FTacCoordinates> GetOccupiedCells(ETacGridLayer Layer, UBattleTeam* Team) const;
	bool IsCellOccupied(FTacCoordinates Coords) const;
	TArray<FTacCoordinates> GetValidPlacementCells(ETacGridLayer Layer) const;
	// Iterates all cells in Layer; adds coords where Predicate returns true
	void FilterCells(ETacGridLayer Layer, TFunctionRef<bool(FTacCoordinates)> Predicate, TArray<FTacCoordinates>& OutCells) const;

	void PushCorpse(AUnit* Unit, FTacCoordinates Coords);
	AUnit* GetTopCorpse(FTacCoordinates Coords) const;
	AUnit* PopCorpse(FTacCoordinates Coords);
	bool HasCorpses(FTacCoordinates Coords) const;
	const TArray<TObjectPtr<AUnit>>& GetCorpseStack(FTacCoordinates Coords) const;

	TArray<AUnit*> GetUnitsInCells(const TArray<FTacCoordinates>& CellCoords, ETacGridLayer Layer) const;

	// Collects units from the specified storage locations
	TArray<AUnit*> GetUnits(EUnitQuerySource Sources) const;
	// Collects units from the specified storage locations, filtered by Predicate
	TArray<AUnit*> GetUnits(EUnitQuerySource Sources, TFunctionRef<bool(const AUnit*)> Predicate) const;

	bool IsMultiCellUnit(const AUnit* Unit) const;
	const FMultiCellUnitData* GetMultiCellData(const AUnit* Unit) const;

	// Primitive spawn: creates actor, places in grid, assigns team. No event binding, no turn registration.
	AUnit* SpawnUnit(TSubclassOf<AUnit> UnitClass, UUnitDefinition* Definition, FTacCoordinates Cell, UBattleTeam* Team);
	// Removes unit from grid data and its team. Does NOT destroy or HandleDeath.
	void RemoveUnitFromGrid(AUnit* Unit);

	// Off-field container: alive units removed from grid due to status (Fleeing etc.), team membership preserved.
	// Deduces cleanup flags from unit's current status (Fleeing -> both true, others -> false).
	void PlaceUnitOffField(AUnit* Unit);
	void PlaceUnitOffField(AUnit* Unit, bool bClearEffects, bool bUnsubscribeAbilities);
	// Returns false if cell is invalid or occupied. checkf's that unit is in off-field container.
	bool ReturnUnitToField(const FGuid& UnitID, FTacCoordinates TargetCoords);
	bool IsUnitOffField(const AUnit* Unit) const;
	TArray<AUnit*> GetOffFieldUnits() const;
private:
#if WITH_EDITOR
	friend class UTacGridEditorInitializer;
#endif
	TArray<FGridRow>& GetLayer(ETacGridLayer Layer);
	const TArray<FGridRow>& GetLayer(ETacGridLayer Layer) const;

	UPROPERTY()
	TArray<FGridRow> GroundLayer;
	UPROPERTY()
	TArray<FGridRow> AirLayer;
	UPROPERTY()
	TMap<FGuid, bool> UnitFlankStates;
	UPROPERTY()
	TMap<FGuid, FRotator> UnitOriginalRotations;
	UPROPERTY()
	TMap<FGuid, FMultiCellUnitData> MultiCellUnits;
	UPROPERTY()
	TMap<FGuid, TObjectPtr<AUnit>> OffFieldUnits;
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;
	UPROPERTY()
	TObjectPtr<UBattleTeam> AttackerTeam;
	UPROPERTY()
	TObjectPtr<UBattleTeam> DefenderTeam;
	bool bPlayerIsAttacker = false;
	FVector GridWorldLocation;
};

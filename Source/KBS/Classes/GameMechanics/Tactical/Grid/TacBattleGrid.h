#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameMechanics/Units/Unit.h"
#include "BattleTeam.h"
#include "GameplayTypes/GridCoordinates.h"
#include "TacBattleGrid.generated.h"

enum class ETargetReach : uint8;
class UDecalComponent;
class UGridDataManager;
class UGridMovementComponent;
class UGridTargetingComponent;
class UGridHighlightComponent;

USTRUCT()
struct FGridRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<AUnit>> Cells;

	FGridRow()
	{
		Cells.Init(nullptr, 5);
	}
};

USTRUCT(BlueprintType)
struct FUnitPlacement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Unit Placement")
	TSubclassOf<AUnit> UnitClass;

	UPROPERTY(EditAnywhere, Category="UnitPlacement")
	bool bIsAttacker = false;

	UPROPERTY(EditAnywhere, Category = "Unit Placement", meta = (ClampMin = "0", ClampMax = "4"))
	int32 Row = 0;

	UPROPERTY(EditAnywhere, Category = "Unit Placement", meta = (ClampMin = "0", ClampMax = "4"))
	int32 Col = 0;

	UPROPERTY(EditAnywhere, Category = "Unit Placement")
	EBattleLayer Layer = EBattleLayer::Ground;
};

UCLASS(BlueprintType)
class KBS_API UGridConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	TObjectPtr<UMaterialInterface> MoveAllowedDecalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	TObjectPtr<UMaterialInterface> EnemyDecalMaterial;

};

UCLASS()
class KBS_API ATacBattleGrid : public AActor
{
	GENERATED_BODY()

public:
	ATacBattleGrid();

	bool IsValidCell(int32 Row, int32 Col, EBattleLayer Layer) const;
	bool PlaceUnit(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer);
	AUnit* GetUnit(int32 Row, int32 Col, EBattleLayer Layer) const;
	bool RemoveUnit(int32 Row, int32 Col, EBattleLayer Layer);
	FVector GetCellWorldLocation(int32 Row, int32 Col, EBattleLayer Layer) const;

	bool IsFlankCell(int32 Row, int32 Col) const;
	bool IsRestrictedCell(int32 Row, int32 Col) const;
	UBattleTeam* GetTeamForUnit(AUnit* Unit) const;
	UBattleTeam* GetEnemyTeam(AUnit* Unit) const;

	TArray<FIntPoint> GetValidMoveCells(AUnit* Unit) const;
	bool MoveUnit(AUnit* Unit, int32 TargetRow, int32 TargetCol);

	TArray<FIntPoint> GetValidTargetCells(AUnit* Unit, ETargetReach Reach) const;
	TArray<AUnit*> GetValidTargetUnits(AUnit* Unit, ETargetReach Reach) const;

	void SelectUnit(AUnit* Unit);
	bool TryMoveSelectedUnit(int32 TargetRow, int32 TargetCol);
	void ClearSelection();
	FIntPoint GetCellFromWorldLocation(FVector WorldLocation) const;
	AUnit* GetSelectedUnit() const { return SelectedUnit; }

	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;

	UFUNCTION()
	void HandleUnitClicked(AUnit* Unit);

	void UnitEntersFlank(AUnit* Unit, int32 Row, int32 Col);
	void UnitExitsFlank(AUnit* Unit);
	void UnitConflict(AUnit* Attacker, AUnit* Defender);

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBoxComponent> GridCollision;

	UPROPERTY(EditAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UGridDataManager> DataManager;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UGridMovementComponent> MovementComponent;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UGridTargetingComponent> TargetingComponent;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UGridHighlightComponent> HighlightComponent;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Teams")
	TObjectPtr<UBattleTeam> AttackerTeam;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Teams")
	TObjectPtr<UBattleTeam> DefenderTeam;

	UPROPERTY(EditAnywhere, Category = "BattleGrid")
	TObjectPtr<UGridConfig> Config;

	UPROPERTY(EditAnywhere, Category = "BattleGrid|Selection")
	TObjectPtr<AUnit> SelectedUnit;

	UPROPERTY(EditAnywhere, Category = "BattleGrid|Editor Setup")
	TArray<FUnitPlacement> EditorUnitPlacements;

	UPROPERTY(EditAnywhere, Category = "BattleGrid|ShowPreview")
	bool bShowPreviewGizmos = true;

	UPROPERTY()
	TArray<TObjectPtr<AUnit>> SpawnedUnits;
};

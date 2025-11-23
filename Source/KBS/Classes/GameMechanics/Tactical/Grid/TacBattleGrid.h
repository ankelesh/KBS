#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameMechanics/Units/Unit.h"
#include "BattleTeam.h"
#include "TacBattleGrid.generated.h"

enum class ETargetReach : uint8;
class UDecalComponent;

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

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	static constexpr int32 GridSize = 5;
	static constexpr int32 TotalCells = 25;
	static constexpr float CellSize = 200.0f;
	static constexpr float AirLayerHeight = 300.0f;
	static constexpr int32 CenterRow = 2;
	static constexpr int32 ExcludedColLeft = 0;
	static constexpr int32 ExcludedColRight = 4;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditAnywhere, Category = "Grid")
	TArray<TObjectPtr<AUnit>> GroundLayer;

	UPROPERTY(EditAnywhere, Category = "Grid")
	TArray<TObjectPtr<AUnit>> AirLayer;

	UPROPERTY(VisibleAnywhere, Category = "Teams")
	TObjectPtr<UBattleTeam> AttackerTeam;

	UPROPERTY(VisibleAnywhere, Category = "Teams")
	TObjectPtr<UBattleTeam> DefenderTeam;

	UPROPERTY(EditAnywhere, Category = "Highlighting")
	TObjectPtr<UMaterialInterface> MoveAllowedDecalMaterial;

	UPROPERTY(EditAnywhere, Category = "Highlighting")
	TObjectPtr<UMaterialInterface> EnemyDecalMaterial;

	UPROPERTY(EditAnywhere, Category = "Selection")
	TObjectPtr<AUnit> SelectedUnit;

	UPROPERTY(EditAnywhere, Category = "Highlighting")
	TArray<TObjectPtr<UDecalComponent>> MoveAllowedDecals;

	UPROPERTY(EditAnywhere, Category = "Highlighting")
	TArray<TObjectPtr<UDecalComponent>> EnemyDecals;

	int32 CoordsToIndex(int32 Row, int32 Col) const;
	TArray<TObjectPtr<AUnit>>& GetLayer(EBattleLayer Layer);
	const TArray<TObjectPtr<AUnit>>& GetLayer(EBattleLayer Layer) const;
};

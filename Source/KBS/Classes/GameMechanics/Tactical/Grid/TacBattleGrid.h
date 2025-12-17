#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "BattleTeam.h"
#include "GameplayTypes/GridCoordinates.h"
#include "TacBattleGrid.generated.h"

enum class ETargetReach : uint8;
class UDecalComponent;
class UGridDataManager;
class UGridMovementComponent;
class UGridTargetingComponent;
class UGridHighlightComponent;
class UTurnManagerComponent;
class UAbilityExecutorComponent;
class UPresentationTrackerComponent;
class UUnitAbilityInstance;

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

	UPROPERTY(EditAnywhere, Category = "Unit Placement")
	TObjectPtr<UUnitDefinition> Definition = nullptr;
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


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentUnitChanged, AUnit*, NewCurrentUnit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitHovered, AUnit*, HoveredUnit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetailsRequested, AUnit*, ClickedUnit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionComplete);

UCLASS()
class KBS_API ATacBattleGrid : public AActor
{
	GENERATED_BODY()

public:
	// ========== Lifecycle ==========
	ATacBattleGrid();

	// ========== Grid Data & Validation ==========
	bool IsValidCell(int32 Row, int32 Col, EBattleLayer Layer) const;
	bool IsFlankCell(int32 Row, int32 Col) const;
	bool IsRestrictedCell(int32 Row, int32 Col) const;
	FVector GetCellWorldLocation(int32 Row, int32 Col, EBattleLayer Layer) const;
	FIntPoint GetCellFromWorldLocation(FVector WorldLocation) const;

	// ========== Unit Placement & Retrieval ==========
	bool PlaceUnit(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer);
	AUnit* GetUnit(int32 Row, int32 Col, EBattleLayer Layer) const;
	bool RemoveUnit(int32 Row, int32 Col, EBattleLayer Layer);
	bool GetUnitPosition(const AUnit* Unit, int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const;

	// ========== Movement API ==========
	TArray<FIntPoint> GetValidMoveCells(AUnit* Unit) const;
	bool MoveUnit(AUnit* Unit, int32 TargetRow, int32 TargetCol);

	// ========== Targeting API ==========
	TArray<FIntPoint> GetValidTargetCells(AUnit* Unit) const;
	TArray<AUnit*> GetValidTargetUnits(AUnit* Unit) const;

	// ========== Selection & Interaction ==========
	void SelectUnit(AUnit* Unit);
	void ClearSelection();
	AUnit* GetSelectedUnit() const { return SelectedUnit; }
	bool TryMoveSelectedUnit(int32 TargetRow, int32 TargetCol);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void SetHoveredUnit(AUnit* Unit);

	// ========== Team Management ==========
	UBattleTeam* GetTeamForUnit(AUnit* Unit) const;
	UBattleTeam* GetEnemyTeam(AUnit* Unit) const;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	TArray<AUnit*> GetTeamUnits(bool bIsAttackerTeam) const;

	// ========== Flank State Management ==========
	void UnitEntersFlank(AUnit* Unit, int32 Row, int32 Col);
	void UnitExitsFlank(AUnit* Unit);
	bool IsUnitOnFlank(const AUnit* Unit) const;
	void SetUnitOnFlank(AUnit* Unit, bool bOnFlank);
	FRotator GetUnitOriginalRotation(const AUnit* Unit) const;
	void SetUnitOriginalRotation(AUnit* Unit, const FRotator& Rotation);

	// ========== Ability System ==========
	void AbilityTargetSelected(AUnit* SourceUnit, const TArray<AUnit*>& Targets);

	UFUNCTION()
	void HandleAbilityEquipped(UUnitAbilityInstance* Ability);

	// ========== Component Accessors ==========
	UFUNCTION(BlueprintCallable, Category="Getters")
	UTurnManagerComponent* GetTurnManager();

	UFUNCTION(BlueprintCallable, Category="Getters")
	UPresentationTrackerComponent* GetPresentationTracker() const;

	// ========== Event Handlers ==========
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void HandleUnitClicked(AUnit* Unit, FKey ButtonPressed);

	UFUNCTION()
	void HandleUnitDied(AUnit* Unit);

	UFUNCTION()
	void HandleBattleEnded(UBattleTeam* Winner);

	UFUNCTION()
	void HandleUnitTurnStart(AUnit* Unit);

	// ========== Actor Overrides ==========
	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;

	// ========== Event Delegates ==========
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCurrentUnitChanged OnCurrentUnitChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitHovered OnUnitHovered;

	UPROPERTY(BlueprintAssignable, Category = "Battle Events")
	FOnActionComplete OnMovementComplete;

	UPROPERTY(BlueprintAssignable, Category = "Battle Events")
	FOnActionComplete OnAbilityComplete;

	UPROPERTY(BlueprintAssignable, Category = "Battle Events")
	FOnDetailsRequested OnDetailsRequested;



protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	// ========== BeginPlay Helper Methods ==========
	void InitializeComponents();
	void SpawnAndPlaceUnits();
	void SetupUnitEventBindings();
	void SetupUnitsInLayer(EBattleLayer Layer);
	void BindUnitEvents(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer);
	void ConfigureTurnManager();
	void StartBattle();

	// ========== OnConstruction Helper Methods ==========
	void DrawUnitPlacements();
	void DrawGridCells();

	// ========== Input Helper Methods ==========
	bool GetCellUnderMouse(int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const;

	// ========== Core Components ==========
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBoxComponent> GridCollision;

	// ========== Grid System Components ==========
	UPROPERTY(EditAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UGridDataManager> DataManager;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UGridMovementComponent> MovementComponent;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UGridTargetingComponent> TargetingComponent;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UGridHighlightComponent> HighlightComponent;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UTurnManagerComponent> TurnManager;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UAbilityExecutorComponent> AbilityExecutor;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UPresentationTrackerComponent> PresentationTracker;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<class UAIControllerComponent> AIController;

	// ========== Team Data ==========
	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Teams")
	TObjectPtr<UBattleTeam> AttackerTeam;

	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Teams")
	TObjectPtr<UBattleTeam> DefenderTeam;

	UPROPERTY(EditAnywhere, Category = "BattleGrid|Teams")
	ETeamSide Player1ControlledTeam = ETeamSide::Attacker;

	// ========== Configuration ==========
	UPROPERTY(EditAnywhere, Category = "BattleGrid")
	TObjectPtr<UGridConfig> Config;

	// ========== Runtime State ==========
	UPROPERTY(EditAnywhere, Category = "BattleGrid|Selection")
	TObjectPtr<AUnit> SelectedUnit;

	UPROPERTY()
	TArray<TObjectPtr<AUnit>> SpawnedUnits;

	// ========== Editor Setup ==========
	UPROPERTY(EditAnywhere, Category = "BattleGrid|Editor Setup")
	TArray<FUnitPlacement> EditorUnitPlacements;

	UPROPERTY(EditAnywhere, Category = "BattleGrid|ShowPreview")
	bool bShowPreviewGizmos = true;
};

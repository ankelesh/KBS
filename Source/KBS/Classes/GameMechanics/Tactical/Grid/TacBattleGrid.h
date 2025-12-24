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
class UGridInputLockComponent;
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
	ATacBattleGrid();
	FVector GetCellWorldLocation(int32 Row, int32 Col, EBattleLayer Layer) const;
	FIntPoint GetCellFromWorldLocation(FVector WorldLocation) const;
	TArray<FIntPoint> GetValidTargetCells(AUnit* Unit) const;
	TArray<AUnit*> GetValidTargetUnits(AUnit* Unit) const;
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void SetHoveredUnit(AUnit* Unit);
	UBattleTeam* GetTeamForUnit(AUnit* Unit) const;
	UBattleTeam* GetEnemyTeam(AUnit* Unit) const;
	void AbilityTargetSelected(AUnit* SourceUnit, const TArray<AUnit*>& Targets);
	void AbilitySelfExecute(AUnit* SourceUnit, UUnitAbilityInstance* Ability);
	UFUNCTION(BlueprintCallable, Category="Getters")
	UTurnManagerComponent* GetTurnManager();
	UFUNCTION(BlueprintCallable, Category="Getters")
	UPresentationTrackerComponent* GetPresentationTracker() const;
	UFUNCTION(BlueprintCallable, Category = "Getters")
	UGridDataManager* GetDataManager();
	UFUNCTION(BlueprintCallable, Category = "Getters")
	UGridInputLockComponent* GetInputLockComponent() const;
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void RequestUnitDetails(AUnit* Unit);
	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;
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
	UFUNCTION()
	void HandleUnitClicked(AUnit* Unit, FKey ButtonPressed);
	UFUNCTION()
	void HandleUnitDied(AUnit* Unit);
	UFUNCTION()
	void HandleBattleEnded(bool bHasWinner, ETeamSide WinningSide);
	UFUNCTION()
	void HandleUnitTurnStart(AUnit* Unit);
	bool IsValidCell(int32 Row, int32 Col, EBattleLayer Layer) const;
	bool IsFlankCell(int32 Row, int32 Col) const;
	bool IsRestrictedCell(int32 Row, int32 Col) const;
	bool PlaceUnit(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer);
	AUnit* GetUnit(int32 Row, int32 Col, EBattleLayer Layer) const;
	bool RemoveUnit(int32 Row, int32 Col, EBattleLayer Layer);
	bool GetUnitPosition(const AUnit* Unit, int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const;
	TArray<AUnit*> GetTeamUnits(bool bIsAttackerTeam) const;
private:
	void InitializeComponents();
	void SpawnAndPlaceUnits();
	void SetupUnitEventBindings();
	void SetupUnitsInLayer(EBattleLayer Layer);
	void BindUnitEvents(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer);
	void ConfigureTurnManager();
	void StartBattle();
	void DrawUnitPlacements();
	void DrawGridCells();
	bool GetCellUnderMouse(int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const;
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
	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UTurnManagerComponent> TurnManager;
	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UAbilityExecutorComponent> AbilityExecutor;
	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UPresentationTrackerComponent> PresentationTracker;
	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<class UAIControllerComponent> AIController;
	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<class UTacGridInputRouter> InputRouter;
	UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
	TObjectPtr<UGridInputLockComponent> InputLockComponent;
	UPROPERTY(EditAnywhere, Category = "BattleGrid|Teams")
	ETeamSide Player1ControlledTeam = ETeamSide::Attacker;
	UPROPERTY(EditAnywhere, Category = "BattleGrid")
	TObjectPtr<UGridConfig> Config;
	UPROPERTY()
	TArray<TObjectPtr<AUnit>> SpawnedUnits;
	UPROPERTY(EditAnywhere, Category = "BattleGrid|Editor Setup")
	TArray<FUnitPlacement> EditorUnitPlacements;
	UPROPERTY(EditAnywhere, Category = "BattleGrid|ShowPreview")
	bool bShowPreviewGizmos = true;
};

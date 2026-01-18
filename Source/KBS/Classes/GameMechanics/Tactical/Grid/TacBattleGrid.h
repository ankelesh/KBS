#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "BattleTeam.h"
#include "GameplayTypes/GridCoordinates.h"
#include "Editor/TacGridEditorInitializer.h"
#include "TacBattleGrid.generated.h"



namespace Tactical {
	class UDecalComponent;
	class UGridDataManager;
	class UGridHighlightComponent;

#if WITH_EDITOR
	class UTacGridEditorInitializer;
	class UGridEditorVisualComponent;
#endif



	UCLASS(BlueprintType)
		class KBS_API UGridConfig : public UPrimaryDataAsset
	{
		GENERATED_BODY()
	public:
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
		TObjectPtr<UMaterialInterface> MoveAllowedDecalMaterial;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
		TObjectPtr<UMaterialInterface> EnemyDecalMaterial;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cells|Sizes")
		float CellSize = 200.0f;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cells|Sizes")
		float AirLayerHeight = 500.0f;
	};

	UCLASS()
		class KBS_API ATacBattleGrid : public AActor
	{
		GENERATED_BODY()
	public:
		ATacBattleGrid();
		virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;

		// Component exposure
		UGridDataManager* GetDataManager() { return DataManager; }

		// EDITOR - public for editor components
		UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Runtime")
		TArray<TObjectPtr<AUnit>> SpawnedUnits;
		UPROPERTY(EditAnywhere, Category = "BattleGrid|Editor Setup")
		TArray<FUnitPlacement> EditorUnitPlacements;
		UPROPERTY(EditAnywhere, Category = "BattleGrid|ShowPreview")
		bool bShowPreviewGizmos = true;
		UPROPERTY(EditAnywhere, Category = "BattleGrid")
		TObjectPtr<UGridConfig> Config;

	protected:
		virtual void BeginPlay() override;
		virtual void OnConstruction(const FTransform& Transform) override;
		UFUNCTION()
		void HandleUnitClicked(AUnit* Unit, FKey ButtonPressed);
	private:
		void InitializeComponents();
		bool GetCellUnderMouse(int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const;

		// Visuals
		UPROPERTY(VisibleAnywhere)
		TObjectPtr<USceneComponent> Root;
		UPROPERTY(VisibleAnywhere)
		TObjectPtr<class UBoxComponent> GridCollision;
		UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
		TObjectPtr<UGridHighlightComponent> HighlightComponent;

		// Storage
		UPROPERTY(EditAnywhere, Category = "BattleGrid|Components")
		TObjectPtr<UGridDataManager> DataManager;
		UPROPERTY(EditAnywhere, Category = "BattleGrid|Teams")
		ETeamSide Player1ControlledTeam = ETeamSide::Attacker;

		// Logic
		UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Components")
		TObjectPtr<class UTacGridInputRouter> InputRouter;


#if WITH_EDITORONLY_DATA
		UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Editor")
		TObjectPtr<UTacGridEditorInitializer> EditorInitializer;
		UPROPERTY(VisibleAnywhere, Category = "BattleGrid|Editor")
		TObjectPtr<UGridEditorVisualComponent> EditorVisuals;
#endif
	};
}; //namespace Tactical
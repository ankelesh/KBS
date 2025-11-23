#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridHighlightComponent.generated.h"

class UDecalComponent;
class UMaterialInterface;
class ATacBattleGrid;
class AUnit;
enum class ETargetReach : uint8;

/**
 * Handles visual feedback and highlighting on the tactical grid
 */
UCLASS()
class KBS_API UGridHighlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGridHighlightComponent();

	/**
	 * Initialize component with grid reference
	 */
	void Initialize(ATacBattleGrid* InGrid, USceneComponent* InRoot,
		UMaterialInterface* InMoveDecalMaterial, UMaterialInterface* InEnemyDecalMaterial);

	/**
	 * Create decal pool (called in BeginPlay)
	 */
	void CreateDecalPool();

	/**
	 * Show valid movement cells for a unit
	 */
	void ShowValidMoves(const TArray<FIntPoint>& ValidCells);

	/**
	 * Show valid target cells for a unit
	 */
	void ShowValidTargets(const TArray<FIntPoint>& TargetCells);

	/**
	 * Clear all highlights
	 */
	void ClearHighlights();

private:
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;

	UPROPERTY()
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> MoveDecalMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> EnemyDecalMaterial;

	UPROPERTY()
	TArray<TObjectPtr<UDecalComponent>> MoveAllowedDecals;

	UPROPERTY()
	TArray<TObjectPtr<UDecalComponent>> EnemyDecals;
};

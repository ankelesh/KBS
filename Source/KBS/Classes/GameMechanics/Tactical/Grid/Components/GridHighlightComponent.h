#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridHighlightComponent.generated.h"
class UDecalComponent;
class UMaterialInterface;
class ATacBattleGrid;
class AUnit;
enum class ETargetReach : uint8;
UCLASS()
class KBS_API UGridHighlightComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UGridHighlightComponent();
	void Initialize(ATacBattleGrid* InGrid, USceneComponent* InRoot,
		UMaterialInterface* InMoveDecalMaterial, UMaterialInterface* InEnemyDecalMaterial);
	void CreateDecalPool();
	void ShowValidMoves(const TArray<FIntPoint>& ValidCells);
	void ShowValidTargets(const TArray<FIntPoint>& TargetCells);
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

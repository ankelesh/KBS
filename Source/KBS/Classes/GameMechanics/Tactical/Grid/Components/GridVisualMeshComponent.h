#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridVisualMeshComponent.generated.h"

class ATacBattleGrid;
class UBoxComponent;
class UProceduralMeshComponent;

UCLASS()
class KBS_API UGridVisualMeshComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UGridVisualMeshComponent();
	void Initialize(ATacBattleGrid* InGrid);

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugDrawBoxes = false;

private:
	void BuildCellBoxes();
	void BuildGroundMesh();
	void BuildAirMesh();

	UPROPERTY()
	TObjectPtr<UProceduralMeshComponent> GroundMesh;
	UPROPERTY()
	TObjectPtr<UProceduralMeshComponent> AirMesh;

	// Indexed [Row * GridSize + Col]
	UPROPERTY()
	TArray<TObjectPtr<UBoxComponent>> GroundCellBoxes;
	UPROPERTY()
	TArray<TObjectPtr<UBoxComponent>> AirCellBoxes;

	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;
};

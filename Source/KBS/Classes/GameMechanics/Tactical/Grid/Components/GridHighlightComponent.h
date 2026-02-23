#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GridHighlightComponent.generated.h"

class UDecalComponent;
class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;
class UGridConfig;
class ATacBattleGrid;

UENUM(BlueprintType)
enum class EHighlightType : uint8
{
	Movement,
	Attack,
	CurrentSelected,
	Friendly,
	COUNT UMETA(Hidden)
};

USTRUCT()
struct FHighlightInstance
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UDecalComponent> Decal;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> Niagara;

	FTacCoordinates Coords;
	EHighlightType Type = EHighlightType::Movement;
	bool bInUse = false;
};

UCLASS()
class KBS_API UGridHighlightComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UGridHighlightComponent();

	void Initialize(USceneComponent* InRoot, UGridConfig* InConfig);
	void CreateHighlightPool();

	// Legacy API (delegates to ShowHighlights)
	void ShowValidMoves(const TArray<FTacCoordinates>& ValidCells);
	void ShowValidTargets(const TArray<FTacCoordinates>& TargetCells);

	// New unified API with deduplication
	void ShowHighlights(const TArray<FTacCoordinates>& Cells, EHighlightType HighlightType);
	void ClearHighlights(EHighlightType HighlightType);
	void ClearAllHighlights();

private:
	UMaterialInterface* GetMaterialForType(EHighlightType Type) const;
	UNiagaraSystem* GetNiagaraSystemForType(EHighlightType Type) const;
	TArray<FTacCoordinates> DeduplicateCoordinates(const TArray<FTacCoordinates>& NewCells, EHighlightType NewType);

	UPROPERTY()
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<UGridConfig> Config;

	// Unified pool of highlight instances (50 total)
	UPROPERTY()
	TArray<FHighlightInstance> HighlightPool;

	const FVector NIAGARA_CELL_OFFSET{0,0,10};
	
	// Tracking active highlights per type
	TMap<EHighlightType, TArray<int32>> ActiveHighlightIndices;
};

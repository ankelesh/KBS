#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridMovementComponent.generated.h"

class AUnit;
class ATacBattleGrid;
class UBattleTeam;
class UGridDataManager;
enum class EBattleLayer : uint8;

USTRUCT()
struct FMovementInterpData
{
	GENERATED_BODY()

	FVector StartLocation = FVector::ZeroVector;
	FVector TargetLocation = FVector::ZeroVector;
	FVector Direction = FVector::ZeroVector;
	float ElapsedTime = 0.0f;
	float Duration = 0.3f;
};

/**
 * Handles unit movement validation and execution on the tactical grid
 */
UCLASS()
class KBS_API UGridMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGridMovementComponent();

	/**
	 * Initialize component with grid reference
	 */
	void Initialize(ATacBattleGrid* InGrid, UGridDataManager* InDataManager);

	/**
	 * Get all valid cells a unit can move to
	 */
	TArray<FIntPoint> GetValidMoveCells(AUnit* Unit) const;

	/**
	 * Move a unit to target cell (handles swapping and flank entry)
	 */
	bool MoveUnit(AUnit* Unit, int32 TargetRow, int32 TargetCol);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;

	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;

	TMap<TObjectPtr<AUnit>, FMovementInterpData> UnitsBeingMoved;

	/**
	 * Get valid adjacent movement cells for ground units
	 */
	void GetAdjacentMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;

	/**
	 * Get special flank movement cells (closest enemy flank only)
	 */
	void GetFlankMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;

	/**
	 * Get all valid cells for air units (can move anywhere valid)
	 */
	void GetAirMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;

	void StartMovementInterpolation(AUnit* Unit, FVector StartLocation, FVector TargetLocation, FVector Direction, float Distance, float Speed);
};

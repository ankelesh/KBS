#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GridDataManager.generated.h"

class ATacBattleGrid;

/**
 * Manages grid data storage and unit placement
 * Owns the 2D arrays for ground and air layers
 */
UCLASS()
class KBS_API UGridDataManager : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Initialize grid layers
	 */
	void Initialize();

	/**
	 * Place a unit in the grid
	 */
	bool PlaceUnit(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer, ATacBattleGrid* Grid);

	/**
	 * Get unit at specified cell
	 */
	AUnit* GetUnit(int32 Row, int32 Col, EBattleLayer Layer) const;

	/**
	 * Remove unit from specified cell
	 */
	bool RemoveUnit(int32 Row, int32 Col, EBattleLayer Layer, ATacBattleGrid* Grid);

	/**
	 * Get layer array (mutable)
	 */
	TArray<FGridRow>& GetLayer(EBattleLayer Layer);

	/**
	 * Get layer array (const)
	 */
	const TArray<FGridRow>& GetLayer(EBattleLayer Layer) const;

private:
	UPROPERTY()
	TArray<FGridRow> GroundLayer;

	UPROPERTY()
	TArray<FGridRow> AirLayer;
};

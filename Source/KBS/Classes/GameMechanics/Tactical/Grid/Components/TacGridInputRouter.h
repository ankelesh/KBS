#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTypes/GridCoordinates.h"
#include "TacGridInputRouter.generated.h"

class ATacBattleGrid;
class UGridDataManager;
enum class ETacGridLayer : uint8;

UCLASS()
class KBS_API UTacGridInputRouter : public UActorComponent
{
	GENERATED_BODY()
public:
	UTacGridInputRouter();
	void Initialize(ATacBattleGrid* InGrid, UGridDataManager* InDataManager);
	void HandleGridClick(FKey ButtonPressed);
private:
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;
	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;

	bool GetCellUnderMouse(FTacCoordinates& OutCell) const;

	// TODO: Add method to TacticalTurnSystem:
	// - void HandleGridCellClicked(FTacCoordinates ClickedCell);
};

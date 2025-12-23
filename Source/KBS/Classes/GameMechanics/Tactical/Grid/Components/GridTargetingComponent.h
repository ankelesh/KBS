#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridTargetingComponent.generated.h"
class AUnit;
class UBattleTeam;
class UGridDataManager;
enum class EBattleLayer : uint8;
enum class ETargetReach : uint8;
UCLASS()
class KBS_API UGridTargetingComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UGridTargetingComponent();
	void Initialize(UGridDataManager* InDataManager);
	TArray<FIntPoint> GetValidTargetCells(AUnit* Unit) const;
	TArray<FIntPoint> GetValidTargetCells(AUnit* Unit, ETargetReach Reach, bool bUseFlankTargeting = false) const;
	TArray<AUnit*> GetValidTargetUnits(AUnit* Unit) const;
	TArray<AUnit*> GetValidTargetUnits(AUnit* Unit, ETargetReach Reach, bool bUseFlankTargeting = false, bool bIncludeDeadUnits = false) const;
	TArray<AUnit*> ResolveTargetsFromClick(AUnit* SourceUnit, FIntPoint ClickedCell, EBattleLayer ClickedLayer, ETargetReach Reach, const struct FAreaShape* AreaShape = nullptr) const;
private:
	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;
	void GetClosestEnemyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;
	void GetFlankTargetCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;
	void GetAnyEnemyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;
	void GetAllFriendlyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;
	void GetAnyFriendlyCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const;
	void GetEmptyCellsOrFriendly(AUnit* Unit, TArray<FIntPoint>& OutCells) const;
	TArray<AUnit*> GetUnitsInArea(FIntPoint CenterCell, EBattleLayer Layer, const struct FAreaShape& AreaShape) const;
};

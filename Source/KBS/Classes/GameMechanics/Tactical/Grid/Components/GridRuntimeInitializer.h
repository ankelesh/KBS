#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/TacticalBattleSetup.h"
#include "GridRuntimeInitializer.generated.h"

class ATacBattleGrid;

UCLASS()
class KBS_API UGridRuntimeInitializer : public UActorComponent
{
	GENERATED_BODY()
public:
	UGridRuntimeInitializer();

	void SpawnFromBattleSetup(const FTacticalBattleSetup& Setup, TSubclassOf<AUnit> DefaultUnitClass);
	void SetupUnitEventBindings();

private:
	ATacBattleGrid* GetGrid() const;
	void SpawnTeamUnits(const TArray<FBattleUnitPlacement>& Units, bool bIsAttacker,
	                    TSubclassOf<AUnit> UnitClass, int32 StartCol);
};

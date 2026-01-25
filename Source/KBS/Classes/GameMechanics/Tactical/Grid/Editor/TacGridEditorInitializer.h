#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameplayTypes/GridCoordinates.h"
#include "TacGridEditorInitializer.generated.h"
USTRUCT(BlueprintType)
struct FUnitPlacement
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Unit Placement")
	TSubclassOf<AUnit> UnitClass;
	UPROPERTY(EditAnywhere, Category = "UnitPlacement")
	bool bIsAttacker = false;
	UPROPERTY(EditAnywhere, Category = "Unit Placement", meta = (ClampMin = "0", ClampMax = "4"))
	int32 Row = 0;
	UPROPERTY(EditAnywhere, Category = "Unit Placement", meta = (ClampMin = "0", ClampMax = "4"))
	int32 Col = 0;
	UPROPERTY(EditAnywhere, Category = "Unit Placement")
	ETacGridLayer Layer = ETacGridLayer::Ground;
	UPROPERTY(EditAnywhere, Category = "Unit Placement")
	TObjectPtr<UUnitDefinition> Definition = nullptr;
};

#if WITH_EDITOR

class ATacBattleGrid;

UCLASS()
class KBS_API UTacGridEditorInitializer : public UActorComponent
{
	GENERATED_BODY()
public:
	UTacGridEditorInitializer();

	void SpawnAndPlaceUnits();
	void SetupUnitEventBindings();

private:
	void SetupUnitsInLayer(ETacGridLayer Layer);
	void BindUnitEvents(AUnit* Unit, int32 Row, int32 Col, ETacGridLayer Layer);

	ATacBattleGrid* GetGrid() const;
};

#endif // WITH_EDITOR
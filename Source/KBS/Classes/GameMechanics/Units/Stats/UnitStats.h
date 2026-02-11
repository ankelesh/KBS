#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Stats/BaseUnitStatTypes.h"
#include "GameMechanics/Units/Stats/UnitDefenseStats.h"
#include "GameMechanics/Units/Stats/UnitHealth.h"
#include "GameMechanics/Units/Stats/UnitStatusContainer.h"
#include "UnitStats.generated.h"

USTRUCT(BlueprintType)
struct FUnitCoreStats
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	FUnitHealth Health {100};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	FUnitStatPercent Initiative {50};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	FUnitStatPercent Accuracy {75};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	FUnitDefenseStats Defense;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	FUnitStatusContainer Status;

	void InitFromBase(const FUnitCoreStats& Template);
};

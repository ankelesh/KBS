#pragma once
#include "CoreMinimal.h"
#include "AbilityBattleContext.generated.h"
class AUnit;
class ATacBattleGrid;
USTRUCT(BlueprintType)
struct FAbilityBattleContext
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AUnit> SourceUnit = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<AUnit>> TargetUnits;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ATacBattleGrid> Grid = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint TargetCell = FIntPoint(-1, -1);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 TargetLayer = 0;
};

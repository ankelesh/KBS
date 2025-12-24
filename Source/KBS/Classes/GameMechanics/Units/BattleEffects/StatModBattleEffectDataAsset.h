#pragma once
#include "CoreMinimal.h"
#include "BattleEffectDataAsset.h"
#include "GameMechanics/Units/UnitStats.h"
#include "GameplayTypes/DamageTypes.h"
#include "StatModBattleEffectDataAsset.generated.h"

UCLASS(BlueprintType)
class KBS_API UStatModBattleEffectDataAsset : public UBattleEffectDataAsset
{
	GENERATED_BODY()
public:
	UStatModBattleEffectDataAsset()
	{
		DamageSource = EDamageSource::Life;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Modification")
	int32 Duration = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Modification")
	FUnitCoreStats StatModifications;
};

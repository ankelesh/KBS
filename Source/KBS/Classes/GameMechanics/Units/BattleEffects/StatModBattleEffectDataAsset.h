#pragma once
#include "CoreMinimal.h"
#include "BattleEffectDataAsset.h"
#include "GameMechanics/Units/Stats/UnitStatDelta.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	int32 Duration = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	FUnitStatDelta Delta;

	// Delta's Combat fields apply only to the owner's weapons whose damage sources include any of these; empty = all weapons.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSet<EDamageSource> WeaponSourceFilter;
};

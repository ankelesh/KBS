#pragma once
#include "CoreMinimal.h"
#include "BattleEffectDataAsset.h"
#include "GameplayTypes/DamageTypes.h"
#include "DOTBattleEffectDataAsset.generated.h"

UCLASS(BlueprintType)
class KBS_API UDOTBattleEffectDataAsset : public UBattleEffectDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DOT Effect")
	int32 Duration = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DOT Effect")
	float EffectMagnitude = 0.0f;
};

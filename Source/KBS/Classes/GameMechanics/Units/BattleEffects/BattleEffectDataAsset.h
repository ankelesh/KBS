#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTypes/DamageTypes.h"
#include "BattleEffect.h"
#include "BattleEffectDataAsset.generated.h"

UCLASS(BlueprintType)
class KBS_API UBattleEffectDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	EDamageSource DamageSource = EDamageSource::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	EEffectTarget EffectTarget = EEffectTarget::Enemy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	int32 Duration = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	float EffectMagnitude = 0.0f;
};

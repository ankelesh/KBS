#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTypes/DamageTypes.h"
#include "BattleEffectDataAsset.generated.h"
class UNiagaraSystem;
UENUM(BlueprintType)
enum class EEffectTarget : uint8
{
	Enemy UMETA(DisplayName = "Enemy"),
	Self UMETA(DisplayName = "Self")
};
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
	FName StackingId = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> AppliedVFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	float VFXDuration = 3.0f;
};

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTypes/EffectTypes.h"
#include "GameplayTypes/DamageTypes.h"
#include "BattleEffectDataAsset.generated.h"
class UNiagaraSystem;
UCLASS(BlueprintType)
class KBS_API UBattleEffectDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// Description
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	FText Name;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	FText Description;
	
	// Mechanics
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	EDamageSource DamageSource = EDamageSource::Physical;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	ETargetReach EffectTarget = ETargetReach::AnyEnemy;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	FName StackingId = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	int32 DefaultDuration = 3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	/// No way to remove / never expires
	bool isImmutable = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	/// Can be removed by abilities
	bool bIsDispellable = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	EEffectType EffectType = EEffectType::Neutral;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	bool bIsAccuracyDependent = false;
	
	// VFX
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> Image;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals|VFX")
	TSoftObjectPtr<UNiagaraSystem> AppliedVFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals|VFX")
	float VFXDuration = 3.0f;
};

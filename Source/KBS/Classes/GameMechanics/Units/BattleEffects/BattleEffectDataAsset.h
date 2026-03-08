#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
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
	
	// Classification
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	FGameplayTagContainer EffectTags;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	EEffectPolarity Polarity = EEffectPolarity::Neutral;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	EDamageSource DamageSource = EDamageSource::Physical;

	// Targeting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	ETargetReach EffectTarget = ETargetReach::AnyEnemy;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	bool bIsAccuracyDependent = true;

	
	// Duration and stacking
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Duration")
	int32 DefaultDuration = 3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Duration")
	bool bIsDispellable = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Duration")
	FName StackingId = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Duration")
	EEffectStackPolicy StackPolicy = EEffectStackPolicy::RefreshOld;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Duration",
	meta = (EditCondition = "StackPolicy == EEffectStackPolicy::Stack", ClampMin = 1))
	int32 MaxStacks = 1;
	
	// VFX
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> Image;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals|VFX")
	TSoftObjectPtr<UNiagaraSystem> AppliedVFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals|VFX")
	float VFXDuration = 3.0f;
};

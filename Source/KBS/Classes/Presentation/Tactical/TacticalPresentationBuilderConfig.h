#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TacticalPresentationBuilderConfig.generated.h"

// Optional override asset for UTacticalPresentationBuilder. Absent config -> actions/actors use their own defaults.
UCLASS(BlueprintType)
class KBS_API UTacticalPresentationBuilderConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId("TacticalPresentationBuilderConfig", GetFName()); }

	// ----- Reaction / death timeouts (seconds) -----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float HitReactionTimeout = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float DeathTimeout = 4.f;

	// ----- Notify names -----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FName ImpactNotifyName = "Impact";

	// ----- Floating text colors -----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingText|Colors")
	FLinearColor DamageColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingText|Colors")
	FLinearColor MissColor = FLinearColor::Gray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingText|Colors")
	FLinearColor ImmuneColor = FLinearColor::Blue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingText|Colors")
	FLinearColor WardedColor = FLinearColor(0.f, 0.8f, 0.8f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingText|Colors")
	FLinearColor EffectSpawnPositiveColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingText|Colors")
	FLinearColor EffectSpawnNegativeColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingText|Colors")
	FLinearColor EffectSpawnNeutralColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingText|Colors")
	FLinearColor EffectEndedColor = FLinearColor::Gray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingText|Colors")
	FLinearColor StatusActivatedColor = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingText|Colors")
	FLinearColor StatusDeactivatedColor = FLinearColor::Gray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingText|Colors")
	FLinearColor UnitSpawnedColor = FLinearColor::White;
};

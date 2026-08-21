#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AbilityPresentationAsset.generated.h"

class UNiagaraSystem;
class USoundBase;
class AProjectilePresentationActor;
class UAbilityStepPresenter;

UENUM(BlueprintType)
enum class EDeliveryTopology : uint8
{
    None            UMETA(DisplayName = "None"),
    OnePerTarget    UMETA(DisplayName = "One Per Target"),
    SingleToPoint   UMETA(DisplayName = "Single To Point"),
    PerTargetVfx    UMETA(DisplayName = "Per-Target VFX"),
};

USTRUCT(BlueprintType)
struct FDeliveryPresentation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delivery")
    EDeliveryTopology Topology = EDeliveryTopology::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delivery",
        meta = (EditCondition = "Topology != EDeliveryTopology::None", EditConditionHides))
    TSoftObjectPtr<UNiagaraSystem> TrailVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delivery",
        meta = (EditCondition = "Topology != EDeliveryTopology::None", EditConditionHides))
    TSoftObjectPtr<UNiagaraSystem> ImpactVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delivery",
        meta = (EditCondition = "Topology != EDeliveryTopology::None", EditConditionHides))
    TSoftObjectPtr<USoundBase> TrailSFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delivery",
        meta = (EditCondition = "Topology != EDeliveryTopology::None", EditConditionHides))
    TSoftObjectPtr<USoundBase> ImpactSFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delivery",
        meta = (EditCondition = "Topology != EDeliveryTopology::None", EditConditionHides, ClampMin = "1.0"))
    float Speed = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delivery",
        meta = (EditCondition = "Topology != EDeliveryTopology::None", EditConditionHides))
    TSubclassOf<AProjectilePresentationActor> ActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delivery",
        meta = (EditCondition = "Topology != EDeliveryTopology::None", EditConditionHides, ClampMin = "0.0"))
    float MaxStaggerDelay = 0.f;

    bool IsEmpty() const { return Topology == EDeliveryTopology::None; }

    // Appends paths of all soft refs that require async preloading.
    void CollectSoftPaths(TArray<FSoftObjectPath>& OutPaths) const;
};

// TODO: move-phase visual overrides (walk/run animation set, speed curve overrides).
USTRUCT(BlueprintType)
struct FMovePresentation
{
    GENERATED_BODY()
};

// Cross-cutting ambient effects (e.g. weather/storm); does not mirror any specific log step type.
USTRUCT(BlueprintType)
struct FAmbientPresentation
{
    GENERATED_BODY()
};

UCLASS(BlueprintType)
class KBS_API UAbilityPresentationAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId("AbilityPresentationAsset", GetFName());
    }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
    FDeliveryPresentation Combat;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
    FMovePresentation Movement;

    // Cross-cutting: ambient effects that do not mirror a log step type.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
    FAmbientPresentation Ambient;

    // Declare only; override mechanism not implemented.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
    TSubclassOf<UAbilityStepPresenter> ConverterOverride;
};

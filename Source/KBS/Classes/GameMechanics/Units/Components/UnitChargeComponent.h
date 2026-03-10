#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "UnitChargeComponent.generated.h"

struct FUnitChargeComponentConfig;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KBS_API UUnitChargeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void InitializeFromConfig(const FUnitChargeComponentConfig& Config);

	void IncrementCharge(const FGameplayTag& Tag, int32 Amount = 1);
	void DecrementCharge(const FGameplayTag& Tag, int32 Amount = 1);
	void RefreshCharge(const FGameplayTag& Tag);
	void DropCharge(const FGameplayTag& Tag);

	int32 GetCharge(const FGameplayTag& Tag) const;

private:
	TMap<FGameplayTag, int32> ChargePool;
	TMap<FGameplayTag, int32> ChargeCaps;
};

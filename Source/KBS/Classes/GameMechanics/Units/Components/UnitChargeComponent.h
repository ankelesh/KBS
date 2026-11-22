#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "UnitChargeComponent.generated.h"

struct FUnitChargeComponentConfig;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChargeChanged, const FGameplayTag&, Tag, int32, NewValue);

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
	int32 GetChargeCap(const FGameplayTag& Tag) const;
	bool IsTracked(const FGameplayTag& Tag) const { return ChargePool.Contains(Tag); }
	bool HasCharge(const FGameplayTag& Tag, int32 Amount = 1) const;
	bool IsFull(const FGameplayTag& Tag) const;
	FText GetChargeName(const FGameplayTag& Tag) const;
	TSoftObjectPtr<UTexture2D> GetChargeIcon(const FGameplayTag& Tag) const;
	TArray<FGameplayTag> GetTrackedTags() const;

	UPROPERTY(BlueprintAssignable, Category = "Charges")
	FOnChargeChanged OnChargeChanged;

private:
	TMap<FGameplayTag, int32> ChargePool;
	TMap<FGameplayTag, int32> ChargeCaps;
	TMap<FGameplayTag, FText> ChargeNames;
	TMap<FGameplayTag, TSoftObjectPtr<UTexture2D>> ChargeIcons;
};

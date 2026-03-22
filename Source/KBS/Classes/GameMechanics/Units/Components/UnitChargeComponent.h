#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameMechanics/Units/Components/Config/UnitChargeComponentConfig.h"
#include "UnitChargeComponent.generated.h"

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

	// Ordered as in config — use to build UI slots
	const TArray<FUnitChargeComponentEntry>& GetOrderedEntries() const { return OrderedEntries; }

	UPROPERTY(BlueprintAssignable, Category = "Charges")
	FOnChargeChanged OnChargeChanged;

private:
	TMap<FGameplayTag, int32> ChargePool;
	TMap<FGameplayTag, int32> ChargeCaps;
	TArray<FUnitChargeComponentEntry> OrderedEntries;
};

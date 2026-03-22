#include "GameMechanics/Units/Components/UnitChargeComponent.h"
#include "GameMechanics/Units/Components/Config/UnitChargeComponentConfig.h"

void UUnitChargeComponent::InitializeFromConfig(const FUnitChargeComponentConfig& Config)
{
	for (const FUnitChargeComponentEntry& Entry : Config.Entries)
	{
		const int32 StartValue = Entry.CounterStart == -1 ? Entry.CounterMax : Entry.CounterStart;
		ChargePool.Add(Entry.Tag, StartValue);
		ChargeCaps.Add(Entry.Tag, Entry.CounterMax);
		ChargeNames.Add(Entry.Tag, Entry.DisplayName);
	}
}

void UUnitChargeComponent::IncrementCharge(const FGameplayTag& Tag, int32 Amount)
{
	int32* Current = ChargePool.Find(Tag);
	checkf(Current, TEXT("UUnitChargeComponent::IncrementCharge - Tag not registered: %s"), *Tag.ToString());
	*Current = FMath::Min(*Current + Amount, ChargeCaps[Tag]);
	OnChargeChanged.Broadcast(Tag, *Current);
}

void UUnitChargeComponent::DecrementCharge(const FGameplayTag& Tag, int32 Amount)
{
	int32* Current = ChargePool.Find(Tag);
	checkf(Current, TEXT("UUnitChargeComponent::DecrementCharge - Tag not registered: %s"), *Tag.ToString());
	*Current = FMath::Max(*Current - Amount, 0);
	OnChargeChanged.Broadcast(Tag, *Current);
}

void UUnitChargeComponent::RefreshCharge(const FGameplayTag& Tag)
{
	int32* Current = ChargePool.Find(Tag);
	checkf(Current, TEXT("UUnitChargeComponent::RefreshCharge - Tag not registered: %s"), *Tag.ToString());
	*Current = ChargeCaps[Tag];
	OnChargeChanged.Broadcast(Tag, *Current);
}

void UUnitChargeComponent::DropCharge(const FGameplayTag& Tag)
{
	int32* Current = ChargePool.Find(Tag);
	checkf(Current, TEXT("UUnitChargeComponent::DropCharge - Tag not registered: %s"), *Tag.ToString());
	*Current = 0;
	OnChargeChanged.Broadcast(Tag, *Current);
}

FText UUnitChargeComponent::GetChargeName(const FGameplayTag& Tag) const
{
	const FText* Name = ChargeNames.Find(Tag);
	return Name ? *Name : FText::GetEmpty();
}

TArray<FGameplayTag> UUnitChargeComponent::GetTrackedTags() const
{
	TArray<FGameplayTag> Tags;
	ChargePool.GetKeys(Tags);
	return Tags;
}

int32 UUnitChargeComponent::GetCharge(const FGameplayTag& Tag) const
{
	const int32* Current = ChargePool.Find(Tag);
	return Current ? *Current : 0;
}

#include "GameMechanics/Units/Components/UnitActionHistoryComponent.h"
#include "GameMechanics/Units/Components/Config/UnitActionHistoryCmpConfig.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Unit.h"

void UUnitActionHistoryComponent::BeginPlay()
{
	Super::BeginPlay();
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	checkf(OwnerUnit, TEXT("UUnitActionHistoryComponent must be attached to AUnit"));
	OwnerUnit->OnUnitAbilityUsed.AddDynamic(this, &UUnitActionHistoryComponent::OnAbilityUsed);
}

void UUnitActionHistoryComponent::InitializeFromConfig(const FUnitActionHistoryCmpConfig& Config)
{
	ExampleSequence = Config.TargetSequence;
}

bool UUnitActionHistoryComponent::LastActionHasTag(const FGameplayTag& Tag) const
{
	return LastActionTags.HasTag(Tag);
}

FGameplayTag UUnitActionHistoryComponent::GetLastActionTag() const
{
	for (auto It = LastActionTags.CreateConstIterator(); It; ++It)
	{
		return *It;
	}
	return FGameplayTag{};
}

bool UUnitActionHistoryComponent::IsSequenceComplete() const
{
	return ExampleSequence.Num() > 0 && CurrentSequence.Num() == ExampleSequence.Num();
}

void UUnitActionHistoryComponent::ResetSequence()
{
	CurrentSequence.Reset();
}

void UUnitActionHistoryComponent::OnAbilityUsed(AUnit* Unit, UUnitAbilityInstance* Ability)
{
	LastActionTags = Ability->GetTags();

	if (IsSequenceComplete()) return;

	const int32 NextIdx = CurrentSequence.Num();
	if (NextIdx >= ExampleSequence.Num()) return;

	for (auto It = LastActionTags.CreateConstIterator(); It; ++It)
	{
		if (*It == ExampleSequence[NextIdx])
		{
			CurrentSequence.Add(*It);
			break;
		}
	}
}

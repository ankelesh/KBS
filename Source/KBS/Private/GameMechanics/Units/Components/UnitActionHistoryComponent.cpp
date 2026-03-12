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

bool UUnitActionHistoryComponent::IsSequenceComplete() const
{
	return ExampleSequence.Num() > 0 && CurrentSequenceStep == ExampleSequence.Num();
}

void UUnitActionHistoryComponent::ResetSequence()
{
	CurrentSequenceStep = 0;
}

void UUnitActionHistoryComponent::OnAbilityUsed(AUnit* Unit, UUnitAbilityInstance* Ability)
{
	LastActionTags = Ability->GetTags();

	if (IsSequenceComplete()) return;
	if (CurrentSequenceStep >= ExampleSequence.Num()) return;

	if (LastActionTags.HasTag(ExampleSequence[CurrentSequenceStep]))
		CurrentSequenceStep++;
}

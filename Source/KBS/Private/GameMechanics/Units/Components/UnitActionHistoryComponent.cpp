#include "GameMechanics/Units/Components/UnitActionHistoryComponent.h"
#include "GameMechanics/Units/Components/Config/UnitActionHistoryCmpConfig.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"

void UUnitActionHistoryComponent::BeginPlay()
{
	Super::BeginPlay();
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	checkf(OwnerUnit, TEXT("UUnitActionHistoryComponent must be attached to AUnit"));
	OwnerUnit->OnUnitAbilityUsed.AddDynamic(this, &UUnitActionHistoryComponent::OnAbilityUsed);

	CombatSubsystem = GetWorld()->GetSubsystem<UTacCombatSubsystem>();
	checkf(CombatSubsystem, TEXT("UTacCombatSubsystem not found"));

	UTacGridSubsystem* GridSubsystem = GetWorld()->GetSubsystem<UTacGridSubsystem>();
	checkf(GridSubsystem, TEXT("UTacGridSubsystem not found"));
	TargetingService = GridSubsystem->GetGridTargetingService();
}

void UUnitActionHistoryComponent::InitializeFromConfig(const FUnitActionHistoryCmpConfig& Config)
{
	ExampleSequence = Config.TargetSequence;

	for (const auto& [Tag, Asset] : Config.TagToDescriptorMap)
	{
		UCombatDescriptor* Descriptor = NewObject<UCombatDescriptor>(this);
		Descriptor->Initialize(this, Asset);
		TagToDescriptorMap.Add(Tag, Descriptor);
	}
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

	for (auto It = LastActionTags.CreateConstIterator(); It; ++It)
	{
		if (auto const* Descriptor = TagToDescriptorMap.Find(*It))
		{
			TArray<AUnit*> Targets = TargetingService->GetValidTargetUnits(Unit, (*Descriptor)->GetTargeting());
			CombatSubsystem->ResolveReactionAttack(Unit, Targets, *Descriptor);
			break;
		}
	}

	if (IsSequenceComplete()) return;
	if (CurrentSequenceStep >= ExampleSequence.Num()) return;

	if (LastActionTags.HasTag(ExampleSequence[CurrentSequenceStep]))
		CurrentSequenceStep++;
}

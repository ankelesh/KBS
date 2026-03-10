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
	TagToDescriptorMap = Config.TagToDescriptorMap;
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

	for (auto It = LastActionTags.CreateConstIterator(); It; ++It)
	{
		if (const TObjectPtr<UCombatDescriptorDataAsset>* Asset = TagToDescriptorMap.Find(*It))
		{
			UCombatDescriptor* Descriptor = NewObject<UCombatDescriptor>(Unit);
			Descriptor->Initialize(Unit, *Asset);

			TArray<AUnit*> Targets = TargetingService->GetValidTargetUnits(Unit, Descriptor->GetReach());
			CombatSubsystem->ResolveReactionAttack(Unit, Targets, Descriptor);
			break;
		}
	}

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

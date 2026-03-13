#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "UnitActionHistoryComponent.generated.h"

struct FUnitActionHistoryCmpConfig;
class AUnit;
class UUnitAbilityInstance;
class UCombatDescriptor;
class UTacCombatSubsystem;
class UTacGridTargetingService;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KBS_API UUnitActionHistoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	void InitializeFromConfig(const FUnitActionHistoryCmpConfig& Config);

	// --- Last Action ---
	bool LastActionHasTag(const FGameplayTag& Tag) const;
	const FGameplayTagContainer& GetLastActionTags() const { return LastActionTags; }

	// --- Sequence ---
	bool IsSequenceComplete() const;
	void ResetSequence();

private:
	UFUNCTION()
	void OnAbilityUsed(AUnit* Unit, UUnitAbilityInstance* Ability);

	FGameplayTagContainer LastActionTags;

	TArray<FGameplayTag> ExampleSequence;
	TArray<FGameplayTag> CurrentSequence;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UCombatDescriptor>> TagToDescriptorMap;

	UPROPERTY()
	TObjectPtr<UTacCombatSubsystem> CombatSubsystem;
	UPROPERTY()
	TObjectPtr<UTacGridTargetingService> TargetingService;
	int32 CurrentSequenceStep = 0;
};

#pragma once
#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "AbilityTypes.generated.h"
class UBattleEffect;
class AUnit;

UENUM(BlueprintType)
enum class EDefaultAbilitySlot : uint8
{
	Attack UMETA(DisplayName = "Attack"),
	Move UMETA(DisplayName = "Move"),
	Wait UMETA(DisplayName = "Wait"),
	Defend UMETA(DisplayName = "Defend"),
	Flee UMETA(DisplayName = "Flee")
};

USTRUCT(BlueprintType)
struct FAbilityResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bInvalidInput = false;

	UPROPERTY(BlueprintReadOnly)
	bool bPresentationRunning = false;

	UPROPERTY(BlueprintReadOnly)
	bool bBattleEnded = false;
};

UENUM()
enum class EAbilityTurnReleasePolicy : uint8
{
	// default, any ability can proceed
	Free UMETA(DisplayName = "Free"),
	// only current ability can proceed, lock must be lifted by ability afterwards
	Locked UMETA(DisplayName = "Locked"),
	// abilities can proceed only if they need to check context
	Conditional UMETA(DisplayName = "Conditional"),
	// abilities can not proceed
	Released UMETA(DisplayName = "Released"),
};

struct FAbilityExecutionResult
{
	bool bSuccess = false;
	EAbilityTurnReleasePolicy Policy = EAbilityTurnReleasePolicy::Free;

	bool IsOk() const { return bSuccess; }
	static FAbilityExecutionResult MakeFail() { return FAbilityExecutionResult{}; }

	static FAbilityExecutionResult MakeOk(EAbilityTurnReleasePolicy InPolicy)
	{
		return FAbilityExecutionResult{true, InPolicy};
	}
};

struct FAbilityContext
{
	FGameplayTagContainer TurnTagContext;
	FGameplayTagContainer PersistentTurnTagContext;
	EAbilityTurnReleasePolicy TurnState;
	EAbilityTurnReleasePolicy StateBeforeLock;

	TMap<FGameplayTag, TVariant<int32, bool, FString>> PersistentStorage;
	TMap<FGameplayTag, TVariant<int32, bool, FString>> TurnOnlyStorage;


	explicit FAbilityContext() : TurnState(EAbilityTurnReleasePolicy::Free), StateBeforeLock(EAbilityTurnReleasePolicy::Free)
	{
	}

	// choose one - if condition ever needed and if not. Both check state
	bool CanAct(const bool bIsCurrentAbility) const;
	bool CanConditionalAct(const bool bIsCurrentAbility, const FGameplayTag& Tag,
	                       const bool bIsPersistent = false) const;

	// Tag querying
	bool HasTurnTag(const FGameplayTag& Tag) const;
	bool HasPersistentTurnTag(const FGameplayTag& Tag) const;
	bool HasTag(const FGameplayTag& Tag, bool bIsPersistent) const;
	bool HasTagAnywhere(const FGameplayTag& Tag) const;

	void AddTag(const FGameplayTag& Tag, bool bIsPersistent);
	void RemoveTag(const FGameplayTag& Tag, bool bIsPersistent);
	void Lock();
	void Unlock();

	// must be called each turn
	void ClearTurnData();
};

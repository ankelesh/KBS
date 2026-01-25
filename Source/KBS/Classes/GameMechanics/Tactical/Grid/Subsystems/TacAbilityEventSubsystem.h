#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TacAbilityEventSubsystem.generated.h"

class AUnit;
class UUnitAbilityInstance;

UENUM(BlueprintType)
enum class EAbilityEventType : uint8
{
	OnUnitAttacked UMETA(DisplayName = "On Unit Attacked"),
	OnUnitDamaged UMETA(DisplayName = "On Unit Damaged"),
	OnUnitAttacks UMETA(DisplayName = "On Unit Attacks")
};

/**
 * Central dispatcher for global battlefield events.
 * Allows passive abilities to observe "any unit" events without N×M subscriptions.
 * Example: Rally Cry ability triggers when ANY allied unit attacks.
 */
UCLASS()
class KBS_API UTacAbilityEventSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void RegisterUnit(AUnit* Unit);
	void UnregisterUnit(AUnit* Unit);
	void RegisterAbility(UUnitAbilityInstance* Ability);
	void UnregisterAbility(UUnitAbilityInstance* Ability);

	// Broadcast global battlefield events
	void BroadcastAnyUnitAttacked(AUnit* Victim, AUnit* Attacker);
	void BroadcastAnyUnitDamaged(AUnit* Victim, AUnit* Attacker);
	void BroadcastAnyUnitAttacks(AUnit* Attacker, AUnit* Target);

private:
	TMap<EAbilityEventType, TArray<UUnitAbilityInstance*>> RegisteredAbilities;
	UPROPERTY()
	TArray<TObjectPtr<AUnit>> RegisteredUnits;
};

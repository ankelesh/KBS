// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UnitAbilitySubsystem.generated.h"

class AUnit;
class UUnitAbilityInstance;

UENUM(BlueprintType)
enum class EAbilityEventType : uint8
{
	OnUnitAttacked UMETA(DisplayName = "On Unit Attacked"),
	OnUnitDamaged UMETA(DisplayName = "On Unit Damaged"),
	OnUnitAttacks UMETA(DisplayName = "On Unit Attacks")
};

UCLASS()
class KBS_API UUnitAbilitySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void RegisterUnit(AUnit* Unit);
	void UnregisterUnit(AUnit* Unit);
	void RegisterAbility(UUnitAbilityInstance* Ability);
	void UnregisterAbility(UUnitAbilityInstance* Ability);

	void BroadcastAnyUnitAttacked(AUnit* Victim, AUnit* Attacker);
	void BroadcastAnyUnitDamaged(AUnit* Victim, AUnit* Attacker);
	void BroadcastAnyUnitAttacks(AUnit* Attacker, AUnit* Target);

private:
	TMap<EAbilityEventType, TArray<UUnitAbilityInstance*>> RegisteredAbilities;

	UPROPERTY()
	TArray<TObjectPtr<AUnit>> RegisteredUnits;
};

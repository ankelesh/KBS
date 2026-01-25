#include "GameMechanics/Tactical/Grid/Subsystems/TacAbilityEventSubsystem.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Unit.h"


void UTacAbilityEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisteredAbilities.Empty();
	RegisteredUnits.Empty();
}
void UTacAbilityEventSubsystem::Deinitialize()
{
	RegisteredAbilities.Empty();
	RegisteredUnits.Empty();
	Super::Deinitialize();
}
void UTacAbilityEventSubsystem::RegisterUnit(AUnit* Unit)
{
	if (!Unit)
	{
		return;
	}
	RegisteredUnits.AddUnique(Unit);
}
void UTacAbilityEventSubsystem::UnregisterUnit(AUnit* Unit)
{
	if (!Unit)
	{
		return;
	}
	RegisteredUnits.Remove(Unit);
}
void UTacAbilityEventSubsystem::RegisterAbility(UUnitAbilityInstance* Ability)
{
	if (!Ability)
	{
		return;
	}
	for (int32 i = 0; i < (int32)EAbilityEventType::OnUnitAttacks + 1; i++)
	{
		EAbilityEventType EventType = (EAbilityEventType)i;
		TArray<UUnitAbilityInstance*>& Abilities = RegisteredAbilities.FindOrAdd(EventType);
		Abilities.AddUnique(Ability);
	}
}
void UTacAbilityEventSubsystem::UnregisterAbility(UUnitAbilityInstance* Ability)
{
	if (!Ability)
	{
		return;
	}
	for (auto& Pair : RegisteredAbilities)
	{
		Pair.Value.Remove(Ability);
	}
}
void UTacAbilityEventSubsystem::BroadcastAnyUnitAttacked(AUnit* Victim, AUnit* Attacker)
{
	TArray<UUnitAbilityInstance*>* Abilities = RegisteredAbilities.Find(EAbilityEventType::OnUnitAttacked);
	if (!Abilities)
	{
		return;
	}

	// Notify all subscribed passive abilities
	// Note: Target cell is not meaningful for event-triggered abilities
	FTacCoordinates EmptyCell;
	for (UUnitAbilityInstance* Ability : *Abilities)
	{
		if (Ability)
		{
			Ability->HandleUnitAttacked(Victim, Attacker);
		}
	}
}

void UTacAbilityEventSubsystem::BroadcastAnyUnitDamaged(AUnit* Victim, AUnit* Attacker)
{
	TArray<UUnitAbilityInstance*>* Abilities = RegisteredAbilities.Find(EAbilityEventType::OnUnitDamaged);
	if (!Abilities)
	{
		return;
	}

	for (UUnitAbilityInstance* Ability : *Abilities)
	{
		if (Ability)
		{
			Ability->HandleUnitDamaged(Victim, Attacker);
		}
	}
}

void UTacAbilityEventSubsystem::BroadcastAnyUnitAttacks(AUnit* Attacker, AUnit* Target)
{
	TArray<UUnitAbilityInstance*>* Abilities = RegisteredAbilities.Find(EAbilityEventType::OnUnitAttacks);
	if (!Abilities)
	{
		return;
	}

	for (UUnitAbilityInstance* Ability : *Abilities)
	{
		if (Ability)
		{
			Ability->HandleUnitAttacks(Attacker, Target);
		}
	}
}


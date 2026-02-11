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

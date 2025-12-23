#include "GameMechanics/Tactical/UnitAbilitySubsystem.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Unit.h"
void UUnitAbilitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisteredAbilities.Empty();
	RegisteredUnits.Empty();
}
void UUnitAbilitySubsystem::Deinitialize()
{
	RegisteredAbilities.Empty();
	RegisteredUnits.Empty();
	Super::Deinitialize();
}
void UUnitAbilitySubsystem::RegisterUnit(AUnit* Unit)
{
	if (!Unit)
	{
		return;
	}
	RegisteredUnits.AddUnique(Unit);
}
void UUnitAbilitySubsystem::UnregisterUnit(AUnit* Unit)
{
	if (!Unit)
	{
		return;
	}
	RegisteredUnits.Remove(Unit);
}
void UUnitAbilitySubsystem::RegisterAbility(UUnitAbilityInstance* Ability)
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
void UUnitAbilitySubsystem::UnregisterAbility(UUnitAbilityInstance* Ability)
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
void UUnitAbilitySubsystem::BroadcastAnyUnitAttacked(AUnit* Victim, AUnit* Attacker)
{
	TArray<UUnitAbilityInstance*>* Abilities = RegisteredAbilities.Find(EAbilityEventType::OnUnitAttacked);
	if (!Abilities)
	{
		return;
	}
	for (UUnitAbilityInstance* Ability : *Abilities)
	{
		if (Ability)
		{
			FAbilityBattleContext Context;
			Context.SourceUnit = Victim;
			Context.TargetUnits.Add(Attacker);
			Ability->TriggerAbility(Context);
		}
	}
}
void UUnitAbilitySubsystem::BroadcastAnyUnitDamaged(AUnit* Victim, AUnit* Attacker)
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
			FAbilityBattleContext Context;
			Context.SourceUnit = Victim;
			Context.TargetUnits.Add(Attacker);
			Ability->TriggerAbility(Context);
		}
	}
}
void UUnitAbilitySubsystem::BroadcastAnyUnitAttacks(AUnit* Attacker, AUnit* Target)
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
			FAbilityBattleContext Context;
			Context.SourceUnit = Attacker;
			Context.TargetUnits.Add(Target);
			Ability->TriggerAbility(Context);
		}
	}
}

#include "GameplayTypes/CombatTypes.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"

void FHitInstance::Reset()
{
	for (auto Ability : InterferingAbilities)
	{
		Ability->HitTriggerCleanup(*this);
	}
	bIsHitCancelled = false;
	Attacker = nullptr;
	Target = nullptr;
}

FHitInstance::FHitInstance(AUnit* TargetUnit, AUnit* AttackerUnit, UWeapon* Weapon)
	: Attacker(AttackerUnit), Target(TargetUnit), bIsHitCancelled(false), InterferingAbilities()
{
}

void FHitInstance::Interfere(UUnitAbilityInstance* Ability, bool bIsCancelled)
{
	InterferingAbilities.Add(Ability);
	bIsHitCancelled |= bIsCancelled;
}

void FHitInstance::CheckCancellation()
{
	bIsHitCancelled |= Target->IsDead() || Attacker->IsDead();
}

FHitInstance::~FHitInstance()
{
	for (auto Ability : InterferingAbilities)
		Ability->HitTriggerCleanup(*this);
}

void FCombatContext::Reset()
{
	Hits.Empty();
	for (auto Ability : InterferingAbilities)
	{
		Ability->AttackTriggerCleanup(*this);
	}
	Attacker = nullptr;
	InterferingAbilities.Empty();
	bIsAttackCancelled = false;
}


FCombatContext::FCombatContext(AUnit* AttackerUnit, UWeapon* Weapon, TArray<AUnit*> Targets, bool bIsReaction)
	:
	  Attacker(AttackerUnit), Hits(), AttackerWeapon(Weapon), InterferingAbilities(), bIsAttackCancelled(false), bIsReactionHit(bIsReaction)
{
	for (auto Target: Targets)
	{
		Hits.Add(FHitInstance(Target,AttackerUnit, Weapon));
	}
}

void FCombatContext::Interfere(UUnitAbilityInstance* Ability, bool bIsCancelled)
{
	InterferingAbilities.Add(Ability);
	bIsAttackCancelled |= bIsCancelled;
}

FCombatContext::~FCombatContext()
{
	Hits.Empty();
	for (auto Ability : InterferingAbilities)
		Ability->AttackTriggerCleanup(*this);
}

void FCombatContext::CheckCancellation()
{
	bIsAttackCancelled |= Attacker->IsDead();
}

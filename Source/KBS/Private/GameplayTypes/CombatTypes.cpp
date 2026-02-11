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
	bIsHitCancelled |= Target->GetStats().Health.IsDead() || Attacker->GetStats().Health.IsDead();
}

FHitInstance::~FHitInstance()
{
	for (auto Ability : InterferingAbilities)
		Ability->HitTriggerCleanup(*this);
}

void FAttackContext::Reset()
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
FAttackContext::FAttackContext(AUnit* AttackerUnit, UWeapon* Weapon, TArray<AUnit*> Targets, bool bIsReaction)
	:
	  Attacker(AttackerUnit), Hits(), AttackerWeapon(Weapon), InterferingAbilities(), bIsAttackCancelled(false), bIsReactionHit(bIsReaction)
{
	for (auto Target: Targets)
	{
		Hits.Add(FHitInstance(Target,AttackerUnit, Weapon));
	}
}

void FAttackContext::Interfere(UUnitAbilityInstance* Ability, bool bIsCancelled)
{
	InterferingAbilities.Add(Ability);
	bIsAttackCancelled |= bIsCancelled;
}

FAttackContext::~FAttackContext()
{
	Hits.Empty();
	for (auto Ability : InterferingAbilities)
		Ability->AttackTriggerCleanup(*this);
}

void FAttackContext::CheckCancellation()
{
	bIsAttackCancelled |= Attacker->GetStats().Health.IsDead();
}

#include "GameplayTypes/CombatTypes.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "GameplayTypes/CombatDescriptorTypes.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"

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

FHitInstance::FHitInstance(AUnit* TargetUnit, AUnit* AttackerUnit, UCombatDescriptor* Descriptor)
	: Attacker(AttackerUnit), Target(TargetUnit), bIsHitCancelled(false), InterferingAbilities()
{
}

void FHitInstance::Interfere(UUnitAbility* Ability, bool bIsCancelled)
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


FCombatContext::FCombatContext(AUnit* AttackerUnit, UCombatDescriptor* Descriptor, TArray<AUnit*> Targets, bool bIsReaction)
	:
	  Attacker(AttackerUnit), Hits(), AttackerDescriptor(Descriptor), InterferingAbilities(), bIsAttackCancelled(false), bIsReactionHit(bIsReaction), MagnitudePolicy(Descriptor->GetMagnitudePolicy())
{
	for (auto Target: Targets)
	{
		Hits.Add(FHitInstance(Target, AttackerUnit, Descriptor));
	}
}

void FCombatContext::Interfere(UUnitAbility* Ability, bool bIsCancelled)
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

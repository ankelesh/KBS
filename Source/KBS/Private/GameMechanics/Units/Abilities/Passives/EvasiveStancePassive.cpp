#include "GameMechanics/Units/Abilities/Passives/EvasiveStancePassive.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/CombatTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogEvasiveStance, Log, All);

void UEvasiveStancePassive::InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner)
{
	Super::InitializeFromDefinition(InDefinition, InOwner);
	ModifierID = FGuid::NewGuid();
	UE_LOG(LogEvasiveStance, Log, TEXT("[EvasiveStance] Initialized on %s (ModifierID=%s)"),
		*InOwner->GetLogName(), *ModifierID.ToString());
}

void UEvasiveStancePassive::Subscribe()
{
	Owner->OnBeingTargetedInCalculation.AddUObject(this, &UEvasiveStancePassive::OnBeingTargeted);
	UE_LOG(LogEvasiveStance, Log, TEXT("[EvasiveStance] %s subscribed to OnBeingTargetedInCalculation"),
		*Owner->GetLogName());
}

void UEvasiveStancePassive::Unsubscribe()
{
	Owner->OnBeingTargetedInCalculation.RemoveAll(this);
	UE_LOG(LogEvasiveStance, Log, TEXT("[EvasiveStance] %s unsubscribed from OnBeingTargetedInCalculation"),
		*Owner->GetLogName());
}

void UEvasiveStancePassive::OnBeingTargeted(FAttackContext& Context, FHitInstance& Hit)
{
	Context.Attacker->GetStats().Accuracy.AddFlatModifier(ModifierID, AccuracyPenalty);
	Hit.Interfere(this);
	UE_LOG(LogEvasiveStance, Log,
		TEXT("[EvasiveStance] %s being targeted by %s — applied %d accuracy penalty (now %d)"),
		*Owner->GetLogName(), *Context.Attacker->GetLogName(),
		AccuracyPenalty, Context.Attacker->GetStats().Accuracy.GetValue());
}

void UEvasiveStancePassive::HitTriggerCleanup(FHitInstance& Hit)
{
	Hit.Attacker->GetStats().Accuracy.RemoveFlatModifier(ModifierID, AccuracyPenalty);
	UE_LOG(LogEvasiveStance, Log,
		TEXT("[EvasiveStance] Cleanup — restored %s accuracy after hit on %s (now %d)"),
		*Hit.Attacker->GetLogName(), *Owner->GetLogName(),
		Hit.Attacker->GetStats().Accuracy.GetValue());
}

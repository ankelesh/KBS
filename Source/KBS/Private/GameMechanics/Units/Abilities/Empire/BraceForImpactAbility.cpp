#include "GameMechanics/Units/Abilities/Empire/BraceForImpactAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"

FAbilityExecutionResult UBraceForImpactAbility::Execute(FTacCoordinates TargetCell)
{
	const FAbilityExecutionResult Result = Super::Execute(TargetCell);
	bCounterArmed = true;
	return Result;
}

void UBraceForImpactAbility::Subscribe()
{
	Super::Subscribe();
	Owner->OnUnitDamaged.AddDynamic(this, &UBraceForImpactAbility::HandleOwnerDamaged);
	Owner->OnUnitTurnStart.AddDynamic(this, &UBraceForImpactAbility::HandleOwnerTurnStart);
}

void UBraceForImpactAbility::Unsubscribe()
{
	Owner->OnUnitDamaged.RemoveDynamic(this, &UBraceForImpactAbility::HandleOwnerDamaged);
	Owner->OnUnitTurnStart.RemoveDynamic(this, &UBraceForImpactAbility::HandleOwnerTurnStart);
	Super::Unsubscribe();
}

void UBraceForImpactAbility::HandleOwnerTurnStart(AUnit* Unit)
{
	bCounterArmed = false;
}

void UBraceForImpactAbility::HandleOwnerDamaged(AUnit* Unit, AUnit* Attacker)
{
	if (!bCounterArmed || Owner->IsDead() || !Owner->GetStats().Status.IsDefending() || Attacker->IsDead())
		return;

	if (GetCombatSubsystem()->IsResolvingReaction())
		return;

	if (TryFireReactionAttack(Attacker, true, nullptr))
		bCounterArmed = false;
}

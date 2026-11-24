#include "GameMechanics/Units/Abilities/Empire/CombinedWarfareAbility.h"
#include "GameMechanics/Units/Unit.h"

void UCombinedWarfareAbility::Subscribe()
{
	Super::Subscribe();
	Owner->OnUnitTurnStart.AddUniqueDynamic(this, &UCombinedWarfareAbility::HandleTurnStarted);
}

void UCombinedWarfareAbility::Unsubscribe()
{
	Owner->OnUnitTurnStart.RemoveDynamic(this, &UCombinedWarfareAbility::HandleTurnStarted);
	Super::Unsubscribe();
}

void UCombinedWarfareAbility::HandleTurnStarted(AUnit* Unit)
{
	LastUsedWeapon.Reset();
}

void UCombinedWarfareAbility::HandleTurnEnd()
{
	LastUsedWeapon.Reset();
	Super::HandleTurnEnd();
}

#include "GameMechanics/Units/Abilities/Empire/PaveesaPassive.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/CombatTypes.h"

void UPaveesaPassive::Subscribe()
{
	Owner->OnCombatResolutionResultApplied.AddUObject(this, &UPaveesaPassive::OnResultApplied);
}

void UPaveesaPassive::Unsubscribe()
{
	Owner->OnCombatResolutionResultApplied.RemoveAll(this);
}

void UPaveesaPassive::OnResultApplied(FCombatContext& Context, FHitInstance& Hit, FDamageResult& Result)
{
	if (Context.MagnitudePolicy != EMagnitudePolicy::Damage)
		return;
	if (Context.Attacker->GetGridMetadata().DistanceTo(Owner->GetGridMetadata()) <= 1)
		return;

	const int32 DamageTaken = FMath::FloorToInt(Result.Damage * 0.5f);
	Result.DamageBlocked += Result.Damage - DamageTaken;
	Result.Damage = DamageTaken;
}

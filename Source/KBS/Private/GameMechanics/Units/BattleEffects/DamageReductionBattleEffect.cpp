#include "GameMechanics/Units/BattleEffects/DamageReductionBattleEffect.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/CombatTypes.h"

void UDamageReductionBattleEffect::Initialize(UBattleEffectDataAsset* InConfig)
{
	Super::Initialize(InConfig);
	Duration = Config->DefaultDuration;
}

void UDamageReductionBattleEffect::OnApplied()
{
	HookHandle = Owner->OnCombatResolutionResultApplied.AddUObject(this, &UDamageReductionBattleEffect::OnDamageApplied);
	LogActivation();
}

void UDamageReductionBattleEffect::OnRemoved()
{
	Owner->OnCombatResolutionResultApplied.Remove(HookHandle);
	LogRemoval();
	Super::OnRemoved();
}

void UDamageReductionBattleEffect::OnDamageApplied(FCombatContext& Context, FHitInstance& Hit, FDamageResult& Result)
{
	if (Context.MagnitudePolicy != EMagnitudePolicy::Damage)
		return;

	const int32 Reduced = Result.Damage * CastChecked<UDamageReductionBattleEffectDataAsset>(Config)->ReductionPercent / 100;
	Result.Damage -= Reduced;
	Result.DamageBlocked += Reduced;
	NotifyOnTriggered();
}

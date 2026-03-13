#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameplayTypes/TargetingDescriptor.h"
#include "GameMechanics/Units/Unit.h"
#include "NiagaraSystem.h"
void UBattleEffect::Initialize(UBattleEffectDataAsset* InConfig)
{
	if (!InConfig)
	{
		return;
	}
	Config = InConfig;
	EffectId = FGuid::NewGuid();

	if (Config->AppliedVFX.IsNull() == false)
	{
		Config->AppliedVFX.LoadSynchronous();
	}
}
EDamageSource UBattleEffect::GetDamageSource() const
{
	return Config ? Config->DamageSource : EDamageSource::Physical;
}
FTargetingDescriptor UBattleEffect::GetEffectTargeting() const
{
	return FTargetingDescriptor::FromReach(Config->EffectTarget);
}
EReapplyDecision UBattleEffect::HandleReapply(UBattleEffect* NewEffect)
{
	return EReapplyDecision::OverrideDuration;
}
void UBattleEffect::OnRemoved()
{
	OnEffectRemoved.Broadcast(this);
}
void UBattleEffect::NotifyOnTriggered()
{
	if (Owner)
	{
		Owner->NotifyEffectTriggered(this);
	}
}

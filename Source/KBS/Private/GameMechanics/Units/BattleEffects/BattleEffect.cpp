#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
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
ETargetReach UBattleEffect::GetEffectTargetReach() const
{
	return Config ? Config->EffectTarget : ETargetReach::AnyEnemy;
}
bool UBattleEffect::HandleReapply(UBattleEffect* NewEffect)
{
	return false;
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

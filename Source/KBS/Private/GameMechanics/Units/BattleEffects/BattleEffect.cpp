#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"

void UBattleEffect::InitializeFromDataAsset(UBattleEffectDataAsset* InConfig)
{
	if (!InConfig)
	{
		return;
	}

	Config = InConfig;
	RemainingTurns = InConfig->Duration;
}

EDamageSource UBattleEffect::GetDamageSource() const
{
	return Config ? Config->DamageSource : EDamageSource::Physical;
}

EEffectTarget UBattleEffect::GetEffectTarget() const
{
	return Config ? Config->EffectTarget : EEffectTarget::Enemy;
}

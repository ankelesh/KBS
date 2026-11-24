#include "GameMechanics/Units/Abilities/Passives/ApplyEffectPassive.h"
#include "GameMechanics/Units/Abilities/Passives/ApplyEffectPassiveDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"

void UApplyEffectPassive::OnBattleStart()
{
	TArray<TInstancedStruct<FTacLogStepBase>> UnloggedSteps;
	ApplyConfiguredEffects(Owner, GetEffectsConfig().Effects, UnloggedSteps);
}

const UApplyEffectPassiveDefinition& UApplyEffectPassive::GetEffectsConfig() const
{
	return *CastChecked<UApplyEffectPassiveDefinition>(Config);
}

#include "GameMechanics/Units/Abilities/Passives/ApplyEffectPassiveDefinition.h"
#include "GameMechanics/Units/Abilities/Passives/ApplyEffectPassive.h"

UApplyEffectPassiveDefinition::UApplyEffectPassiveDefinition()
{
	AbilityClass = UApplyEffectPassive::StaticClass();
	bIsPassive = true;
	Targeting = ETargetReach::Self;
}

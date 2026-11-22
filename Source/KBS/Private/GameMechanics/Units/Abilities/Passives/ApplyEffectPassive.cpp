#include "GameMechanics/Units/Abilities/Passives/ApplyEffectPassive.h"
#include "GameMechanics/Units/Abilities/Passives/ApplyEffectPassiveDefinition.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacSubsystemControl.h"

void UApplyEffectPassive::Subscribe()
{
	UTacSubsystemControl* Control = Owner->GetWorld()->GetSubsystem<UTacSubsystemControl>();
	checkf(Control, TEXT("UApplyEffectPassive: TacSubsystemControl unavailable"));
	if (Control->HasBattleStarted())
	{
		ApplyEffects();
	}
	else
	{
		Control->ReadyForStart.AddUniqueDynamic(this, &UApplyEffectPassive::ApplyEffects);
	}
}

void UApplyEffectPassive::Unsubscribe()
{
	UTacSubsystemControl* Control = Owner->GetWorld()->GetSubsystem<UTacSubsystemControl>();
	checkf(Control, TEXT("UApplyEffectPassive: TacSubsystemControl unavailable"));
	Control->ReadyForStart.RemoveDynamic(this, &UApplyEffectPassive::ApplyEffects);
}

void UApplyEffectPassive::ApplyEffects()
{
	if (bEffectsApplied)
	{
		return;
	}
	bEffectsApplied = true;

	for (const FDescriptorEffectConfig& EffectConfig : GetEffectsConfig().Effects)
	{
		checkf(EffectConfig.EffectClass && EffectConfig.EffectConfig,
		       TEXT("UApplyEffectPassive %s: effect entry needs both class and config"), *Config->AbilityName);
		UBattleEffect* Effect = NewObject<UBattleEffect>(Owner, EffectConfig.EffectClass);
		Effect->Initialize(EffectConfig.EffectConfig);
		Effect->PrepareForApply(Owner, Owner);
		Owner->ApplyEffect(Effect);
	}
}

const UApplyEffectPassiveDefinition& UApplyEffectPassive::GetEffectsConfig() const
{
	return *CastChecked<UApplyEffectPassiveDefinition>(Config);
}

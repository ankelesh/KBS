#include "GameMechanics/Units/BattleEffects/StatModBattleEffect.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/StatModBattleEffectDataAsset.h"

void UStatModBattleEffect::Initialize(UBattleEffectDataAsset* InConfig)
{
	Super::Initialize(InConfig);
	UStatModBattleEffectDataAsset* StatModConfig = Cast<UStatModBattleEffectDataAsset>(InConfig);
	if (StatModConfig)
	{
		Duration = StatModConfig->Duration;
	}
}

void UStatModBattleEffect::OnTurnEnd()
{
	DecrementDuration();
	UE_LOG(LogTemp, Log, TEXT("%s: StatMod effect '%s' tick (%d turns remaining)"),
		*Owner->GetName(),
		*Config->Name.ToString(),
		Duration);
}

void UStatModBattleEffect::OnApplied()
{
	if (!Owner)
	{
		return;
	}
	ApplyStatModifications();
	UE_LOG(LogTemp, Log, TEXT("%s: StatMod effect '%s' applied for %d turns"),
		*Owner->GetName(),
		*Config->Name.ToString(),
		Duration);
}

void UStatModBattleEffect::OnRemoved()
{
	if (!Owner)
	{
		return;
	}
	RemoveStatModifications();
	UE_LOG(LogTemp, Log, TEXT("%s: StatMod effect '%s' removed"),
		*Owner->GetName(),
		*Config->Name.ToString());
}

EReapplyDecision UStatModBattleEffect::HandleReapply(UBattleEffect* NewEffect)
{
	if (!NewEffect)
	{
		return EReapplyDecision::DoNothing;
	}
	return EReapplyDecision::New;
}

void UStatModBattleEffect::ApplyStatModifications()
{
	UStatModBattleEffectDataAsset* Cfg = GetStatModConfig();
	if (!Owner || !Cfg)
	{
		return;
	}

	AppliedDelta = Cfg->Delta;
	Owner->GetStats().ApplyDelta(AppliedDelta, GetEffectId());
}

void UStatModBattleEffect::RemoveStatModifications()
{
	if (!Owner)
	{
		return;
	}

	Owner->GetStats().RemoveDelta(AppliedDelta, GetEffectId());
	AppliedDelta.Reset();
}

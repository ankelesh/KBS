#include "GameMechanics/Units/BattleEffects/TargetDOTBattleEffect.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/DOTBattleEffectDataAsset.h"
#include "GameplayTypes/CombatTypes.h"
void UTargetDOTBattleEffect::Initialize(UBattleEffectDataAsset* InConfig, UWeapon* InWeapon, AUnit* InAttacker)
{
	Super::Initialize(InConfig, InWeapon, InAttacker);
	UDOTBattleEffectDataAsset* DOTConfig = Cast<UDOTBattleEffectDataAsset>(InConfig);
	if (DOTConfig)
	{
		RemainingTurns = DOTConfig->Duration;
	}
}
void UTargetDOTBattleEffect::OnTurnEnd(AUnit* Owner)
{
	UDOTBattleEffectDataAsset* DOTConfig = GetDOTConfig();
	if (!Owner || !DOTConfig)
	{
		return;
	}
	FDamageResult Damage;
	Damage.Damage = FMath::RoundToInt(DOTConfig->EffectMagnitude);
	Damage.DamageSource = Config->DamageSource;
	Damage.DamageBlocked = 0;
	Owner->TakeHit(Damage);
	SpawnEffectVFX(Owner);
	UE_LOG(LogTemp, Log, TEXT("%s: DOT effect '%s' dealt %d damage (%d turns remaining)"),
		*Owner->GetName(),
		*Config->Name.ToString(),
		Damage.Damage,
		RemainingTurns);
	DecrementDuration();
}
void UTargetDOTBattleEffect::OnApplied(AUnit* Owner)
{
	SpawnEffectVFX(Owner);
	UE_LOG(LogTemp, Log, TEXT("%s: DOT effect '%s' applied for %d turns"),
		*Owner->GetName(),
		*Config->Name.ToString(),
		RemainingTurns);
}
void UTargetDOTBattleEffect::OnRemoved(AUnit* Owner)
{
	UE_LOG(LogTemp, Log, TEXT("%s: DOT effect '%s' removed"),
		*Owner->GetName(),
		*Config->Name.ToString());
}
bool UTargetDOTBattleEffect::HandleReapply(UBattleEffect* NewEffect, AUnit* Owner)
{
	UDOTBattleEffectDataAsset* DOTConfig = GetDOTConfig();
	UDOTBattleEffectDataAsset* NewDOTConfig = NewEffect ? Cast<UDOTBattleEffectDataAsset>(NewEffect->GetConfig()) : nullptr;
	if (!NewEffect || !DOTConfig || !NewDOTConfig)
	{
		return false;
	}
	float NewMagnitude = NewDOTConfig->EffectMagnitude;
	float CurrentMagnitude = DOTConfig->EffectMagnitude;
	if (NewMagnitude > CurrentMagnitude)
	{
		Config = NewDOTConfig;
		RemainingTurns = NewDOTConfig->Duration;
		SpawnEffectVFX(Owner);
		UE_LOG(LogTemp, Log, TEXT("%s: DOT effect '%s' replaced (new magnitude %.1f > old %.1f), duration reset to %d"),
			*Owner->GetName(),
			*Config->Name.ToString(),
			NewMagnitude,
			CurrentMagnitude,
			RemainingTurns);
		return true;
	}
	RemainingTurns = DOTConfig->Duration;
	SpawnEffectVFX(Owner);
	UE_LOG(LogTemp, Log, TEXT("%s: DOT effect '%s' refreshed (magnitude %.1f <= %.1f), duration reset to %d"),
		*Owner->GetName(),
		*Config->Name.ToString(),
		NewMagnitude,
		CurrentMagnitude,
		RemainingTurns);
	return true;
}

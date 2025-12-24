#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
void UBattleEffect::Initialize(UBattleEffectDataAsset* InConfig, UWeapon* InWeapon, AUnit* InAttacker)
{
	if (!InConfig)
	{
		return;
	}
	Config = InConfig;
	SourceWeapon = InWeapon;
	Attacker = InAttacker;

	if (Config->AppliedVFX.IsNull() == false)
	{
		Config->AppliedVFX.LoadSynchronous();
	}
}
EDamageSource UBattleEffect::GetDamageSource() const
{
	return Config ? Config->DamageSource : EDamageSource::Physical;
}
EEffectTarget UBattleEffect::GetEffectTarget() const
{
	return Config ? Config->EffectTarget : EEffectTarget::Enemy;
}
bool UBattleEffect::HandleReapply(UBattleEffect* NewEffect, AUnit* Owner)
{
	return false;
}
void UBattleEffect::SpawnEffectVFX(AUnit* Owner)
{
	if (!Owner || !Config || !Config->AppliedVFX.Get())
	{
		return;
	}
	UUnitVisualsComponent* VisualsComponent = Owner->GetVisualsComponent();
	if (!VisualsComponent)
	{
		return;
	}
	VisualsComponent->SpawnNiagaraEffect(
		Config->AppliedVFX.Get(),
		Owner->GetActorLocation() + FVector(0, 0, Owner->GetSimpleCollisionHalfHeight()),
		Config->VFXDuration
	);
}

#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "NiagaraComponent.h"
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
void UBattleEffect::SpawnEffectVFX()
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

#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameplayTypes/TargetingDescriptor.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
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
void UBattleEffect::LogRemoval(TArray<TInstancedStruct<FTacLogStepBase>>&& Steps) const
{
	checkf(Owner, TEXT("UBattleEffect::LogRemoval called without Owner set"));
	UTacLogSubsystem* LogSub = Owner->GetWorld()->GetSubsystem<UTacLogSubsystem>();
	checkf(LogSub, TEXT("UTacLogSubsystem missing"));
	FGuid EventId = LogSub->OpenEvent(ETacLogEventType::EffectEnd, ETacLogEventOrigin::Triggered,
	                                  Owner->GetUnitID(), LogSub->FindLatestEvent());
	FEffectEndPayload Payload;
	Payload.EffectAssetId    = Config->GetPrimaryAssetId();
	Payload.EffectInstanceId = EffectId;
	Payload.OwnerUnitId      = Owner->GetUnitID();
	Payload.RemovalReason    = IsExpired() ? EEffectRemovalReason::Expired
		: Owner->IsDead() ? EEffectRemovalReason::OwnerDied : EEffectRemovalReason::Dispelled;
	Payload.Steps            = MoveTemp(Steps);
	LogSub->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FEffectEndPayload>(MoveTemp(Payload)));
}

void UBattleEffect::LogActivation(TArray<TInstancedStruct<FTacLogStepBase>>&& Steps) const
{
	checkf(Owner, TEXT("UBattleEffect::LogActivation called without Owner set"));
	UTacLogSubsystem* LogSub = Owner->GetWorld()->GetSubsystem<UTacLogSubsystem>();
	checkf(LogSub, TEXT("UTacLogSubsystem missing"));
	FGuid EventId = LogSub->OpenEvent(ETacLogEventType::EffectActivation, ETacLogEventOrigin::Triggered,
	                                  Owner->GetUnitID(), LogSub->FindLatestEvent());
	FTacEffectPayload Payload;
	Payload.EffectAssetId    = Config->GetPrimaryAssetId();
	Payload.EffectInstanceId = EffectId;
	Payload.OwnerUnitId      = Owner->GetUnitID();
	Payload.Steps            = MoveTemp(Steps);
	LogSub->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FTacEffectPayload>(MoveTemp(Payload)));
}

void UBattleEffect::NotifyOnTriggered()
{
	if (Owner)
	{
		Owner->NotifyEffectTriggered(this);
	}
}

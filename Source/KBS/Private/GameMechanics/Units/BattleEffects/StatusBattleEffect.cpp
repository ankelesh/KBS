#include "GameMechanics/Units/BattleEffects/StatusBattleEffect.h"
#include "GameMechanics/Units/BattleEffects/StatusBattleEffectDataAsset.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"

void UStatusBattleEffect::Initialize(UBattleEffectDataAsset* InConfig)
{
	Super::Initialize(InConfig);
	const UStatusBattleEffectDataAsset& Cfg = GetStatusConfig();
	Duration = Cfg.bPermanent ? MAX_int32 : Cfg.DefaultDuration;
}

void UStatusBattleEffect::OnApplied()
{
	const EUnitStatus Status = GetStatusConfig().Status;
	Owner->GetStats().Status.AddStatus(Status, EffectId);

	UTacLogSubsystem* LogSub = Owner->GetWorld()->GetSubsystem<UTacLogSubsystem>();
	checkf(LogSub, TEXT("UTacLogSubsystem missing"));
	const FGuid EventId = LogSub->OpenEvent(ETacLogEventType::EffectActivation, ETacLogEventOrigin::Triggered,
	                                        Owner->GetUnitID(), LogSub->FindLatestEvent());
	FTacEffectPayload Payload;
	Payload.EffectAssetId    = Config->GetPrimaryAssetId();
	Payload.EffectInstanceId = EffectId;
	Payload.OwnerUnitId      = Owner->GetUnitID();
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogStatusChangeStep>(
		FTacLogStatusChangeStep::Make(Owner->GetUnitID(), Status, true, EffectId)));
	LogSub->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FTacEffectPayload>(MoveTemp(Payload)));
}

void UStatusBattleEffect::OnRemoved()
{
	const EUnitStatus Status = GetStatusConfig().Status;
	Owner->GetStats().Status.RemoveStatus(Status, EffectId);

	UTacLogSubsystem* LogSub = Owner->GetWorld()->GetSubsystem<UTacLogSubsystem>();
	checkf(LogSub, TEXT("UTacLogSubsystem missing"));
	const FGuid EventId = LogSub->OpenEvent(ETacLogEventType::EffectEnd, ETacLogEventOrigin::Triggered,
	                                        Owner->GetUnitID(), LogSub->FindLatestEvent());
	FEffectEndPayload Payload;
	Payload.EffectAssetId    = Config->GetPrimaryAssetId();
	Payload.EffectInstanceId = EffectId;
	Payload.OwnerUnitId      = Owner->GetUnitID();
	Payload.RemovalReason    = IsExpired() ? EEffectRemovalReason::Expired : EEffectRemovalReason::Dispelled;
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogStatusChangeStep>(
		FTacLogStatusChangeStep::Make(Owner->GetUnitID(), Status, false, EffectId)));
	LogSub->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FEffectEndPayload>(MoveTemp(Payload)));

	Super::OnRemoved();
}

void UStatusBattleEffect::OnTurnEnd()
{
	if (!GetStatusConfig().bPermanent)
	{
		DecrementDuration();
	}
}

const UStatusBattleEffectDataAsset& UStatusBattleEffect::GetStatusConfig() const
{
	return *CastChecked<UStatusBattleEffectDataAsset>(Config);
}

#include "GameMechanics/Units/BattleEffects/StatModBattleEffect.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/StatModBattleEffectDataAsset.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"

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

	UTacLogSubsystem* LogSub = Owner->GetWorld()->GetSubsystem<UTacLogSubsystem>();
	checkf(LogSub, TEXT("UTacLogSubsystem missing"));
	FGuid EventId = LogSub->OpenEvent(ETacLogEventType::EffectActivation, ETacLogEventOrigin::Triggered,
	                                  Owner->GetUnitID(), LogSub->FindLatestEvent());
	FTacEffectPayload Payload;
	Payload.EffectAssetId    = Config->GetPrimaryAssetId();
	Payload.EffectInstanceId = EffectId;
	Payload.OwnerUnitId      = Owner->GetUnitID();
	Payload.SourceId         = FGuid();
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogStatAltStep>(
		FTacLogStatAltStep::Make(FGuid(), Owner->GetUnitID(), AppliedDelta,
		                         EStatModifierRemovalPolicy::DurationControlled)));
	LogSub->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FTacEffectPayload>(MoveTemp(Payload)));
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

	if (!IsExpired()) return;

	UTacLogSubsystem* LogSub = Owner->GetWorld()->GetSubsystem<UTacLogSubsystem>();
	checkf(LogSub, TEXT("UTacLogSubsystem missing"));
	FGuid EventId = LogSub->OpenEvent(ETacLogEventType::EffectEnd, ETacLogEventOrigin::Triggered,
	                                  Owner->GetUnitID(), LogSub->FindLatestEvent());
	FEffectEndPayload Payload;
	Payload.EffectAssetId    = Config->GetPrimaryAssetId();
	Payload.EffectInstanceId = EffectId;
	Payload.OwnerUnitId      = Owner->GetUnitID();
	Payload.RemovalReason    = EEffectRemovalReason::Expired;
	LogSub->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FEffectEndPayload>(MoveTemp(Payload)));
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

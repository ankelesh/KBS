#include "GameMechanics/Units/BattleEffects/TargetDOTBattleEffect.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/DOTBattleEffectDataAsset.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameplayTypes/CombatTypes.h"

void UTargetDOTBattleEffect::Initialize(UBattleEffectDataAsset* InConfig)
{
	Super::Initialize(InConfig);
	UDOTBattleEffectDataAsset* DOTConfig = GetDOTConfig();
	if (DOTConfig)
	{
		Duration = DOTConfig->Duration;
		TickDescriptor = NewObject<UCombatDescriptor>(this);
		TickDescriptor->Initialize(this, DOTConfig->TickDescriptor);
	}
}

void UTargetDOTBattleEffect::PrepareForApply(AUnit* Source, AUnit* Target)
{
	if (Source && TickDescriptor)
		StoredHitChance = FDamageCalculation::CalculateHitChance(Source, TickDescriptor, Target);
}

void UTargetDOTBattleEffect::OnTurnEnd()
{
	checkf(Owner, TEXT("TargetDOTBattleEffect::OnTurnEnd called without Owner"));
	checkf(TickDescriptor, TEXT("TargetDOTBattleEffect::OnTurnEnd: TickDescriptor not initialized"));

	UTacCombatSubsystem* CombatSub = Owner->GetWorld()->GetSubsystem<UTacCombatSubsystem>();
	UTacLogSubsystem* LogSub = Owner->GetWorld()->GetSubsystem<UTacLogSubsystem>();
	checkf(CombatSub, TEXT("UTacCombatSubsystem missing"));
	checkf(LogSub, TEXT("UTacLogSubsystem missing"));

	FCombatHitResult HitResult = CombatSub->ResolveEffectTick(Owner, TickDescriptor, StoredHitChance);
	NotifyOnTriggered();

	UE_LOG(LogTemp, Log, TEXT("%s: DOT effect '%s' ticked — %d dmg (%d turns remaining)"),
		*Owner->GetName(), *Config->Name.ToString(), HitResult.DamageResult.Damage, Duration);

	FGuid EventId = LogSub->OpenEvent(ETacLogEventType::EffectActivation, ETacLogEventOrigin::Triggered,
	                                  Owner->GetUnitID(), LogSub->FindLatestEvent());
	FTacEffectPayload Payload;
	Payload.EffectAssetId    = Config->GetPrimaryAssetId();
	Payload.EffectInstanceId = EffectId;
	Payload.OwnerUnitId      = Owner->GetUnitID();
	Payload.SourceId         = FGuid();
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogCombatStep>(
		FTacLogCombatStep::Make(FGuid(), FTacCoordinates(), Owner->GetUnitID(),
		                        TArray<FCombatHitResult>{HitResult}, FGameplayTag())));
	FTacLogStatusChangeStep::AppendDefendingClearedFromHits(Payload.Steps, TArray<FCombatHitResult>{HitResult});
	LogSub->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FTacEffectPayload>(MoveTemp(Payload)));

	DecrementDuration();
}

void UTargetDOTBattleEffect::OnApplied()
{
	checkf(Owner, TEXT("TargetDOTBattleEffect::OnApplied called without Owner set"));
	NotifyOnTriggered();
	UE_LOG(LogTemp, Log, TEXT("%s: DOT effect '%s' applied for %d turns"),
		*Owner->GetName(), *Config->Name.ToString(), Duration);
}

void UTargetDOTBattleEffect::OnRemoved()
{
	checkf(Owner, TEXT("TargetDOTBattleEffect::OnRemoved called without Owner set"));
	UE_LOG(LogTemp, Log, TEXT("%s: DOT effect '%s' removed"),
		*Owner->GetName(), *Config->Name.ToString());

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

EReapplyDecision UTargetDOTBattleEffect::HandleReapply(UBattleEffect* NewEffect)
{
	if (!NewEffect) return EReapplyDecision::DoNothing;
	UTargetDOTBattleEffect* NewDOT = Cast<UTargetDOTBattleEffect>(NewEffect);
	if (!NewDOT) return EReapplyDecision::DoNothing;
	checkf(Owner, TEXT("TargetDOTBattleEffect::HandleReapply called without Owner set"));
	const int32 NewMagnitude     = NewDOT->TickDescriptor->GetStats().BaseMagnitude.GetValue();
	const int32 CurrentMagnitude = TickDescriptor->GetStats().BaseMagnitude.GetValue();
	return NewMagnitude > CurrentMagnitude ? EReapplyDecision::New : EReapplyDecision::OverrideDuration;
}

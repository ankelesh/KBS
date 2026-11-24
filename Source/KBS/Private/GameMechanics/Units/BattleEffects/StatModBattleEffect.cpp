#include "GameMechanics/Units/BattleEffects/StatModBattleEffect.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Combat/Weapon.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
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

	LogRemoval();
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
	ApplyWeaponModifications();
}

void UStatModBattleEffect::ApplyWeaponModifications()
{
	if (AppliedDelta.DamageSourcesToAdd.IsEmpty() && AppliedDelta.MagnitudeFlat == 0 && AppliedDelta.MagnitudeMultiplier == 0
		&& !AppliedDelta.bGrantsGuaranteedHit)
	{
		return;
	}

	const TSet<EDamageSource>& Filter = GetStatModConfig()->WeaponSourceFilter;
	for (UWeapon* Weapon : Owner->GetWeapons())
	{
		UCombatDescriptor* Descriptor = Weapon->GetDescriptor();
		if (!Filter.IsEmpty() && Filter.Intersect(Descriptor->GetStats().DamageSources.GetValue()).IsEmpty())
		{
			continue;
		}
		if (!AppliedDelta.DamageSourcesToAdd.IsEmpty())
		{
			Descriptor->ModifySource(AppliedDelta.DamageSourcesToAdd, GetEffectId());
		}
		if (AppliedDelta.MagnitudeFlat != 0)
		{
			Descriptor->ModifyMagnitude(AppliedDelta.MagnitudeFlat, GetEffectId(), true);
		}
		if (AppliedDelta.MagnitudeMultiplier != 0)
		{
			Descriptor->ModifyMagnitude(AppliedDelta.MagnitudeMultiplier, GetEffectId(), false);
		}
		if (AppliedDelta.bGrantsGuaranteedHit)
		{
			Descriptor->GrantGuaranteedHit(GetEffectId());
		}
	}
}

void UStatModBattleEffect::RemoveWeaponModifications()
{
	for (UWeapon* Weapon : Owner->GetWeapons())
	{
		UCombatDescriptor* Descriptor = Weapon->GetDescriptor();
		Descriptor->RemoveSourceModifier(GetEffectId());
		Descriptor->RemoveGuaranteedHitGrant(GetEffectId());
		if (AppliedDelta.MagnitudeFlat != 0)
		{
			Descriptor->RemoveMagnitudeModifier(AppliedDelta.MagnitudeFlat, GetEffectId(), true);
		}
		if (AppliedDelta.MagnitudeMultiplier != 0)
		{
			Descriptor->RemoveMagnitudeModifier(AppliedDelta.MagnitudeMultiplier, GetEffectId(), false);
		}
	}
}

void UStatModBattleEffect::RemoveStatModifications()
{
	if (!Owner)
	{
		return;
	}

	RemoveWeaponModifications();
	Owner->GetStats().RemoveDelta(AppliedDelta, GetEffectId());
	AppliedDelta.Reset();
}

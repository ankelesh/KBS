#include "GameMechanics/Units/Abilities/Empire/ServeAndProtectPassive.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/CombatTypes.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"

void UServeAndProtectPassive::InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner)
{
	Super::InitializeFromDefinition(InDefinition, InOwner);
	ModifierID = FGuid::NewGuid();
}

void UServeAndProtectPassive::Subscribe()
{
	UTacCombatSubsystem* Combat = GetCombatSubsystem();
	checkf(Combat, TEXT("UServeAndProtectPassive: TacCombatSubsystem unavailable"));
	Combat->OnCalculationPhase.AddUniqueDynamic(this, &UServeAndProtectPassive::OnCalculationPhase);
}

void UServeAndProtectPassive::Unsubscribe()
{
	UTacCombatSubsystem* Combat = GetCombatSubsystem();
	checkf(Combat, TEXT("UServeAndProtectPassive: TacCombatSubsystem unavailable"));
	Combat->OnCalculationPhase.RemoveDynamic(this, &UServeAndProtectPassive::OnCalculationPhase);
}

void UServeAndProtectPassive::OnCalculationPhase(FCombatContext& Context, FHitInstance& Hit)
{
	if (Context.MagnitudePolicy != EMagnitudePolicy::Damage || !IsStandingBehindOwner(Hit.Target))
		return;

	const FUnitArmour& OwnerArmour = Owner->GetStats().Defense.Armour;
	FUnitArmour& TargetArmour = Hit.Target->GetStats().Defense.Armour;
	FUnitStatDelta Delta;
	for (const TPair<EDamageSource, int32>& Entry : OwnerArmour.GetBaseArmour())
	{
		const int32 Gap = OwnerArmour.GetValue(Entry.Key) - TargetArmour.GetValue(Entry.Key);
		if (Gap <= 0)
			continue;
		TargetArmour.AddFlatModifier(ModifierID, Gap, Entry.Key);
		Delta.ArmourDeltas.Add(Entry.Key, Gap);
	}
	if (Delta.IsEmpty())
		return;

	// One registration per covered unit: its cleanup removes everything added, even by later hits.
	const bool bFirstCover = !AppliedCover.Contains(Hit.Target);
	TMap<EDamageSource, int32>& Cover = AppliedCover.FindOrAdd(Hit.Target);
	for (const TPair<EDamageSource, int32>& Entry : Delta.ArmourDeltas)
		Cover.FindOrAdd(Entry.Key) += Entry.Value;
	if (bFirstCover)
		Hit.Interfere(this);

	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(LogSubsystem);
	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogStatAltStep>(
		FTacLogStatAltStep::Make(Owner->GetUnitID(), Hit.Target->GetUnitID(), Delta,
		                         EStatModifierRemovalPolicy::InstaRemove)));
	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::Ability, ETacLogEventOrigin::Reaction,
	                                        Owner->GetUnitID(), LogSubsystem->FindLatestEvent());
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));
}

void UServeAndProtectPassive::HitTriggerCleanup(FHitInstance& Hit)
{
	TMap<EDamageSource, int32> Applied;
	const bool bHadCover = AppliedCover.RemoveAndCopyValue(Hit.Target, Applied);
	// Every cleanup matches exactly one registration; a miss means a hit copy or a repeated cleanup.
	ensureMsgf(bHadCover, TEXT("ServeAndProtect (%s): cleanup for %s found no cover - hit cleaned up twice or out of order"),
	           *Owner->GetLogName(), *Hit.Target->GetLogName());
	if (!bHadCover)
		return;

	FUnitArmour& TargetArmour = Hit.Target->GetStats().Defense.Armour;
	for (const TPair<EDamageSource, int32>& Entry : Applied)
		TargetArmour.RemoveFlatModifier(ModifierID, Entry.Value, Entry.Key);
}

bool UServeAndProtectPassive::IsStandingBehindOwner(const AUnit* Unit) const
{
	const FUnitGridMetadata& OwnerMeta = Owner->GetGridMetadata();
	const FUnitGridMetadata& UnitMeta = Unit->GetGridMetadata();
	if (Unit == Owner || !OwnerMeta.IsOnField() || OwnerMeta.bOnFlank || !UnitMeta.IsOnField()
		|| !OwnerMeta.IsAlly(UnitMeta))
		return false;

	// A 2-cell owner's extra cell is its rear cell, so cover starts one cell past that.
	const FTacCoordinates RearCell = OwnerMeta.IsMultiCell() ? OwnerMeta.ExtraCell : OwnerMeta.Coords;
	return UnitMeta.GetCells().Contains(GetExtraCellCoords(RearCell, OwnerMeta.Orientation));
}

#include "GameMechanics/Units/Abilities/Empire/ArmourPenetrationPassive.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/CombatTypes.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"

void UArmourPenetrationPassive::InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner)
{
	Super::InitializeFromDefinition(InDefinition, InOwner);
	ModifierID = FGuid::NewGuid();
}

void UArmourPenetrationPassive::Subscribe()
{
	Owner->OnResolutionCalculating.AddUObject(this, &UArmourPenetrationPassive::OnCalculating);
}

void UArmourPenetrationPassive::Unsubscribe()
{
	Owner->OnResolutionCalculating.RemoveAll(this);
}

void UArmourPenetrationPassive::OnCalculating(FCombatContext& Context, FHitInstance& Hit)
{
	FUnitArmour& TargetArmour = Hit.Target->GetStats().Defense.Armour;
	const int32 ArmourBefore = TargetArmour.GetValue(EDamageSource::Physical);
	TargetArmour.AddMultiplier(ModifierID, ArmourMultiplierPercent, EDamageSource::Physical);
	Hit.Interfere(this);

	FUnitStatDelta Delta;
	Delta.ArmourDeltas.Add(EDamageSource::Physical, TargetArmour.GetValue(EDamageSource::Physical) - ArmourBefore);

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

void UArmourPenetrationPassive::HitTriggerCleanup(FHitInstance& Hit)
{
	Hit.Target->GetStats().Defense.Armour.RemoveMultiplier(ModifierID, ArmourMultiplierPercent, EDamageSource::Physical);
}

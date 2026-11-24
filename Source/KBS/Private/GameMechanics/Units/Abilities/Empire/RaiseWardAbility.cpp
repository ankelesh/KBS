#include "GameMechanics/Units/Abilities/Empire/RaiseWardAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Stats/UnitInstantChange.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"

FAbilityExecutionResult URaiseWardAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(LogSubsystem);

	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::Ability, ETacLogEventOrigin::Initiated,
	                                        Owner->GetUnitID(), FGuid());

	Owner->GetStats().Status.SetDefending();

	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Command           = TargetCell;
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogStatusChangeStep>(
		FTacLogStatusChangeStep::Make(Owner->GetUnitID(), EUnitStatus::Defending, true)));

	const TSet<EDamageSource>& CurrentWards = Owner->GetStats().Defense.Wards.GetWards();
	static const EDamageSource AllSources[] = {
		EDamageSource::Fire, EDamageSource::Earth, EDamageSource::Air, EDamageSource::Water, EDamageSource::Death};
	TArray<EDamageSource> Candidates;
	for (const EDamageSource Source : AllSources)
	{
		if (!CurrentWards.Contains(Source))
			Candidates.Add(Source);
	}
	if (!Candidates.IsEmpty())
	{
		FUnitInstantChange Change;
		Change.WardsToGrant.Add(Candidates[FMath::RandHelper(Candidates.Num())]);
		const int32 HealApplied = Owner->ApplyInstantChange(Change);
		Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogInstantChangeStep>(
			FTacLogInstantChangeStep::Make(Owner->GetUnitID(), HealApplied, Owner->GetStats().Health.GetCurrent(),
			                               Change.WardsToGrant)));
	}
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));

	ConsumeUse();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

#include "Presentation/Tactical/TacticalPresentationBuilder.h"
#include "Presentation/Core/PresentationSequence.h"
#include "Presentation/Core/Actions/MovePresentationAction.h"
#include "Presentation/Tactical/Converters/MoveStepConverter.h"
#include "Presentation/Tactical/Converters/FleeStepConverter.h"
#include "Presentation/Tactical/Converters/EffectSpawnStepConverter.h"
#include "Presentation/Tactical/Converters/StatusChangeStepConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"

void UTacticalPresentationBuilder::SetGridDataManager(UGridDataManager* InGridDataManager)
{
	GridDataManager = InGridDataManager;
}

void UTacticalPresentationBuilder::SetLogSubsystem(UTacLogSubsystem* InLogSubsystem)
{
	LogSubsystem = InLogSubsystem;
}

namespace
{
	// Steps live inside payloads that carry a Steps array (Ability use, effect activation).
	// Other payload types (turn change, unit spawn/exit, effect end) don't carry steps yet.
	const TArray<TInstancedStruct<FTacLogStepBase>>* GetStepsFromPayload(const TInstancedStruct<FTacLogPayload>& Payload)
	{
		if (const FAbilityUsePayload* Ability = Payload.GetPtr<FAbilityUsePayload>())
		{
			return &Ability->Steps;
		}
		if (const FTacEffectPayload* Effect = Payload.GetPtr<FTacEffectPayload>())
		{
			return &Effect->Steps;
		}

		// TODO: convert FTurnChangePayload, FUnitSpawnPayload, FUnitMoveOffFieldPayload, FEffectEndPayload
		// once corresponding presentation actions exist.
		return nullptr;
	}
}

UPresentationSequence* UTacticalPresentationBuilder::Build_Implementation(FGuid FromEventId, FGuid ToEventId)
{
	checkf(GridDataManager && LogSubsystem, TEXT("TacticalPresentationBuilder requires GridDataManager and LogSubsystem to be set before Build"));

	const TArray<FGuid>& Spine = LogSubsystem->GetSpine();

	int32 StartIndex = 0;
	if (FromEventId.IsValid())
	{
		const int32 FromIndex = Spine.IndexOfByKey(FromEventId);
		checkf(FromIndex != INDEX_NONE, TEXT("TacticalPresentationBuilder: FromEventId not found on spine"));
		StartIndex = FromIndex + 1;
	}

	const int32 EndIndex = Spine.IndexOfByKey(ToEventId);
	checkf(EndIndex != INDEX_NONE, TEXT("TacticalPresentationBuilder: ToEventId not found on spine"));

	TMap<FGuid, AUnit*> UnitLookup;
	for (AUnit* Unit : GridDataManager->GetUnits(EUnitQuerySource::OnField | EUnitQuerySource::OffField))
	{
		UnitLookup.Add(Unit->GetUnitID(), Unit);
	}

	UPresentationSequence* Sequence = NewObject<UPresentationSequence>();

	for (int32 SpineIndex = StartIndex; SpineIndex <= EndIndex; ++SpineIndex)
	{
		const FTacLogEvent* Event = LogSubsystem->GetEvent(Spine[SpineIndex]);
		checkf(Event, TEXT("Spine entry has no matching event in the log"));

		const TArray<TInstancedStruct<FTacLogStepBase>>* Steps = GetStepsFromPayload(Event->Payload);
		if (!Steps)
		{
			continue;
		}

		for (const TInstancedStruct<FTacLogStepBase>& Step : *Steps)
		{
			if (const FTacLogMoveStep* MoveStep = Step.GetPtr<FTacLogMoveStep>())
			{
				Sequence->Actions.Add(TacticalLogConverters::ConvertMoveStep(*MoveStep, UnitLookup, GridDataManager));
				continue;
			}

			if (const FTacLogFleeStep* FleeStep = Step.GetPtr<FTacLogFleeStep>())
			{
				Sequence->Actions.Add(TacticalLogConverters::ConvertFleeStep(*FleeStep, UnitLookup, GridDataManager));
				continue;
			}

			if (const FTacLogEffectSpawnStep* EffectSpawnStep = Step.GetPtr<FTacLogEffectSpawnStep>())
			{
				Sequence->Actions.Add(TacticalLogConverters::ConvertEffectSpawnStep(*EffectSpawnStep, UnitLookup));
				continue;
			}

			if (const FTacLogStatusChangeStep* StatusChangeStep = Step.GetPtr<FTacLogStatusChangeStep>())
			{
				Sequence->Actions.Add(TacticalLogConverters::ConvertStatusChangeStep(*StatusChangeStep, UnitLookup));
				continue;
			}

			if (Step.GetPtr<FTacLogWaitStep>())
			{
				// TODO: no presentation yet - Wait has nothing to visualize today.
				continue;
			}

			if (Step.GetPtr<FTacLogStatAltStep>())
			{
				// TODO: no presentation yet - visualized via the accompanying EffectSpawnStep instead.
				continue;
			}

			// TODO: convert FTacLogCombatStep once a damage-number presentation action exists.
		}
	}

	return Sequence;
}

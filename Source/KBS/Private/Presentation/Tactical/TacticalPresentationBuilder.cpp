#include "Presentation/Tactical/TacticalPresentationBuilder.h"
#include "Presentation/KBSPresentationLog.h"
#include "Presentation/PresentationBuildContext.h"
#include "Presentation/Assets/AbilityPresentationAsset.h"
#include "Presentation/Tactical/TacticalPresentationBuilderConfig.h"
#include "Presentation/Core/PresentationSequence.h"
#include "Presentation/Core/Actions/MovePresentationAction.h"
#include "Presentation/Tactical/Converters/MoveStepConverter.h"
#include "Presentation/Tactical/Converters/FleeStepConverter.h"
#include "Presentation/Tactical/Converters/EffectSpawnStepConverter.h"
#include "Presentation/Tactical/Converters/StatusChangeStepConverter.h"
#include "Presentation/Tactical/Converters/CombatStepConverter.h"
#include "Presentation/Tactical/Converters/UnitSpawnPayloadConverter.h"
#include "Presentation/Tactical/Converters/UnitMoveOffFieldPayloadConverter.h"
#include "Presentation/Tactical/Converters/EffectEndPayloadConverter.h"
#include "Presentation/Tactical/Converters/TacEffectPayloadConverter.h"
#include "Presentation/Tactical/Converters/UnitDespawnPayloadConverter.h"
#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"
#include "Presentation/Tactical/Actions/DespawnPresentationAction.h"
#include "Presentation/Core/Actions/VfxPresentationAction.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "Engine/AssetManager.h"

void UTacticalPresentationBuilder::SetGridSubsystem(UTacGridSubsystem* InGridSubsystem)
{
	GridSubsystem = InGridSubsystem;
}

void UTacticalPresentationBuilder::SetLogSubsystem(UTacLogSubsystem* InLogSubsystem)
{
	LogSubsystem = InLogSubsystem;
}

void UTacticalPresentationBuilder::SetConfig(UTacticalPresentationBuilderConfig* InConfig)
{
	Config = InConfig;
}

void UTacticalPresentationBuilder::BeginPreload(const TArray<FPrimaryAssetId>& AbilityAssetIds, FSimpleDelegate OnComplete)
{
	// Phase 1: collect manifest soft paths from ability assets.
	// Ability assets are already loaded synchronously by the simulation layer (unchanged per spec).
	TArray<FSoftObjectPath> ManifestPaths;
	for (const FPrimaryAssetId& Id : AbilityAssetIds)
	{
		const UUnitAbilityDefinition* AbilityDef = Cast<UUnitAbilityDefinition>(
			UAssetManager::Get().GetPrimaryAssetObject(Id));
		if (AbilityDef && !AbilityDef->Presentation.IsNull())
		{
			ManifestPaths.AddUnique(AbilityDef->Presentation.ToSoftObjectPath());
		}
	}

	if (ManifestPaths.IsEmpty())
	{
		OnComplete.ExecuteIfBound();
		return;
	}

	FStreamableManager& SM = UAssetManager::Get().GetStreamableManager();

	// Phase 1 async load: manifest assets.
	ManifestHandle = SM.RequestAsyncLoad(ManifestPaths,
		[this, ManifestPaths, OnComplete]()
		{
			// Phase 2: collect inner soft refs (VFX/SFX) from newly-loaded manifests.
			TArray<FSoftObjectPath> ContentPaths;
			for (const FSoftObjectPath& Path : ManifestPaths)
			{
				if (const UAbilityPresentationAsset* Manifest = Cast<UAbilityPresentationAsset>(Path.ResolveObject()))
				{
					Manifest->Combat.CollectSoftPaths(ContentPaths);
				}
			}

			if (ContentPaths.IsEmpty())
			{
				OnComplete.ExecuteIfBound();
				return;
			}

			FStreamableManager& SM2 = UAssetManager::Get().GetStreamableManager();
			ContentHandle = SM2.RequestAsyncLoad(ContentPaths,
				[OnComplete]() { OnComplete.ExecuteIfBound(); });
		});
}

namespace
{
	// Steps live inside payloads that carry a Steps array (ability use, effect activation).
	// Other payload types (turn change, unit spawn/exit, effect end) carry no Steps array -
	// they're converted directly from the payload in the main loop below instead.
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
		if (const FEffectEndPayload* EffectEnd = Payload.GetPtr<FEffectEndPayload>())
		{
			return &EffectEnd->Steps;
		}
		return nullptr;
	}

	// Resolves the presentation manifest for an ability-use payload.
	// Ability assets are synchronously loaded (unchanged per spec); manifest was preloaded.
	// Returns nullptr if the ability has no manifest assigned or the asset is unloaded.
	UAbilityPresentationAsset* ResolveManifest(const FAbilityUsePayload& AbilityPayload)
	{
		// AbilityAssetId type is "AbilityDefinition"; the simulation loads these synchronously
		// before the log event is written, so GetPrimaryAssetObject resolves without a disk load.
		const UUnitAbilityDefinition* AbilityDef = Cast<UUnitAbilityDefinition>(
			UAssetManager::Get().GetPrimaryAssetObject(AbilityPayload.AbilityAssetId));
		if (!AbilityDef)
		{
			return nullptr;
		}
		// Manifest was preloaded asynchronously via BeginPreload; Get() resolves synchronously here.
		return AbilityDef->Presentation.Get();
	}
}

UPresentationSequence* UTacticalPresentationBuilder::Build_Implementation(FGuid FromEventId, FGuid ToEventId)
{
	checkf(GridSubsystem && LogSubsystem, TEXT("TacticalPresentationBuilder requires GridSubsystem and LogSubsystem to be set before Build"));

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

	// Corpses included - a killed unit is pushed off the grid into corpse storage synchronously
	// during combat resolution, before its combat step's log event even closes. PendingDespawn included
	// for the same reason - a despawned unit is resolvable only there until presentation finalizes it.
	TMap<FGuid, AUnit*> UnitLookup;
	for (AUnit* Unit : GridSubsystem->GetAllUnits())
	{
		UnitLookup.Add(Unit->GetUnitID(), Unit);
	}

	const bool bDump = CVarDumpSequence.GetValueOnGameThread() != 0;

	UPresentationSequence* Sequence = NewObject<UPresentationSequence>();

	for (int32 SpineIndex = StartIndex; SpineIndex <= EndIndex; ++SpineIndex)
	{
		const FGuid EventId = Spine[SpineIndex];
		const FTacLogEvent* Event = LogSubsystem->GetEvent(EventId);
		checkf(Event, TEXT("Spine entry has no matching event in the log"));

		// Assemble context once per payload, before the step loop.
		FPresentationBuildContext Context;
		Context.UnitLookup   = &UnitLookup;
		Context.GridSubsystem = GridSubsystem;
		Context.Config        = Config;
		Context.EventId       = EventId;

		// Resolve ability payload and manifest for ability-use events.
		if (const FAbilityUsePayload* AbilityPayload = Event->Payload.GetPtr<FAbilityUsePayload>())
		{
			Context.AbilityPayload = AbilityPayload;
			Context.Manifest       = ResolveManifest(*AbilityPayload);
		}

		// Payload-level conversion: independent of Steps, since a payload (e.g. FEffectEndPayload) can
		// carry both a direct visual (floating text) and a Steps array driving further per-step visuals.
		if (const FUnitSpawnPayload* SpawnPayload = Event->Payload.GetPtr<FUnitSpawnPayload>())
		{
			Sequence->Actions.Add(TacticalLogConverters::ConvertUnitSpawnPayload(*SpawnPayload, Context));
		}
		else if (const FUnitMoveOffFieldPayload* MoveOffPayload = Event->Payload.GetPtr<FUnitMoveOffFieldPayload>())
		{
			Sequence->Actions.Add(TacticalLogConverters::ConvertUnitMoveOffFieldPayload(*MoveOffPayload, Context));
		}
		else if (const FEffectEndPayload* EffectEndPayload = Event->Payload.GetPtr<FEffectEndPayload>())
		{
			// No backing asset (e.g. an innate status expiring, not a real effect) - nothing to name, skip.
			if (UFloatingTextPresentationAction* Action = TacticalLogConverters::ConvertEffectEndPayload(*EffectEndPayload, Context))
			{
				Sequence->Actions.Add(Action);
			}
		}
		else if (const FUnitDespawnPayload* DespawnPayload = Event->Payload.GetPtr<FUnitDespawnPayload>())
		{
			Sequence->Actions.Add(TacticalLogConverters::ConvertUnitDespawnPayload(*DespawnPayload, Context));
		}
		// FTurnChangePayload: TODO - no presentation yet, will drive HUD once that exists.

		if (const FTacEffectPayload* EffectPayload = Event->Payload.GetPtr<FTacEffectPayload>())
		{
			if (UVfxPresentationAction* Vfx = TacticalLogConverters::ConvertTacEffectPayload(*EffectPayload, Context))
			{
				Sequence->Actions.Add(Vfx);
			}
		}

		const TArray<TInstancedStruct<FTacLogStepBase>>* Steps = GetStepsFromPayload(Event->Payload);
		if (!Steps)
		{
			continue;
		}

		for (const TInstancedStruct<FTacLogStepBase>& Step : *Steps)
		{
			if (const FTacLogMoveStep* MoveStep = Step.GetPtr<FTacLogMoveStep>())
			{
				Sequence->Actions.Add(TacticalLogConverters::ConvertMoveStep(*MoveStep, Context));
				continue;
			}

			if (const FTacLogFleeStep* FleeStep = Step.GetPtr<FTacLogFleeStep>())
			{
				Sequence->Actions.Add(TacticalLogConverters::ConvertFleeStep(*FleeStep, Context));
				continue;
			}

			if (const FTacLogEffectSpawnStep* EffectSpawnStep = Step.GetPtr<FTacLogEffectSpawnStep>())
			{
				Sequence->Actions.Add(TacticalLogConverters::ConvertEffectSpawnStep(*EffectSpawnStep, Context));
				continue;
			}

			if (const FTacLogStatusChangeStep* StatusChangeStep = Step.GetPtr<FTacLogStatusChangeStep>())
			{
				Sequence->Actions.Add(TacticalLogConverters::ConvertStatusChangeStep(*StatusChangeStep, Context));
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

			if (const FTacLogCombatStep* CombatStep = Step.GetPtr<FTacLogCombatStep>())
			{
				TacticalLogConverters::ConvertCombatStep(*CombatStep, Context, Sequence->Actions);
				continue;
			}
		}
	}

	const int32 TotalActions = Sequence->Actions.Num();

	if (bDump)
	{
		UE_LOG(LogKBSPresentation, Log,
			TEXT("[PresentationDump] Build complete: %d actions assembled, %d slots skipped"),
			TotalActions, SkippedSlots);

		for (int32 i = 0; i < TotalActions; ++i)
		{
			if (const UPresentationSequenceAction* Action = Sequence->Actions[i])
			{
				UE_LOG(LogKBSPresentation, Log,
					TEXT("[PresentationDump]   [%d] %s"),
					i, *Action->GetClass()->GetName());
			}
		}
	}

	return Sequence;
}

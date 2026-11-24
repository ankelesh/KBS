#include "Presentation/Tactical/Converters/AbilityUsePayloadConverter.h"
#include "Presentation/Tactical/TacticalPresentationBuilderConfig.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"
#include "Engine/AssetManager.h"
#include "Algo/AllOf.h"

namespace
{
	bool IsRoutineMovement(const FAbilityUsePayload& Payload)
	{
		if (Payload.Steps.IsEmpty())
			return false;
		return Algo::AllOf(Payload.Steps, [](const TInstancedStruct<FTacLogStepBase>& Step)
		{
			return Step.GetPtr<FTacLogMoveStep>() || Step.GetPtr<FTacLogWaitStep>();
		});
	}
}

UFloatingTextPresentationAction* TacticalLogConverters::ConvertAbilityUsePayload(const FAbilityUsePayload& Payload,
	const FGuid& InstigatorId, const FPresentationBuildContext& Context)
{
	if (IsRoutineMovement(Payload))
		return nullptr;

	AUnit* const* FoundUnit = Context.UnitLookup->Find(InstigatorId);
	checkf(FoundUnit, TEXT("Ability use payload references unit %s - unit is present in UnitLookup"), *InstigatorId.ToString());

	// Asset must already be loaded - the ability was created from it during real gameplay.
	const UUnitAbilityDefinition* Definition = Cast<UUnitAbilityDefinition>(
		UAssetManager::Get().GetPrimaryAssetObject(Payload.AbilityAssetId));
	if (!Definition || Definition->AbilityName.IsEmpty())
		return nullptr;

	UFloatingTextPresentationAction* Action = NewObject<UFloatingTextPresentationAction>();
	Action->Context.Actor = *FoundUnit;
	Action->Context.Text = FText::FromString(Definition->AbilityName);
	Action->Context.Color = Context.Config ? Context.Config->AbilityUsedColor : FLinearColor::White;
	return Action;
}

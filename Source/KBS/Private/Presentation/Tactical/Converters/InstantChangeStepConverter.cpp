#include "Presentation/Tactical/Converters/InstantChangeStepConverter.h"
#include "Presentation/Tactical/TacticalPresentationBuilderConfig.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Units/Unit.h"
#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"

void TacticalLogConverters::ConvertInstantChangeStep(const FTacLogInstantChangeStep& Step,
                                                     const FPresentationBuildContext& Context,
                                                     TArray<TObjectPtr<UPresentationSequenceAction>>& OutActions)
{
	AUnit* const* FoundUnit = Context.UnitLookup->Find(Step.UnitId);
	checkf(FoundUnit, TEXT("Instant change step references unit %s - unit is present in UnitLookup"), *Step.UnitId.ToString());

	const FLinearColor HealColor = Context.Config ? Context.Config->HealColor : FLinearColor::Green;
	const FLinearColor WardColor = Context.Config ? Context.Config->WardedColor : FLinearColor(0.f, 0.8f, 0.8f);

	auto AddLabel = [&](const FText& Text, const FLinearColor& Color)
	{
		UFloatingTextPresentationAction* Action = NewObject<UFloatingTextPresentationAction>();
		Action->Context.Actor = *FoundUnit;
		Action->Context.Text = Text;
		Action->Context.Color = Color;
		OutActions.Add(Action);
	};

	if (Step.HealApplied > 0)
		AddLabel(FText::FromString(FString::Printf(TEXT("+%d"), Step.HealApplied)), HealColor);

	for (const EDamageSource Source : Step.WardsGranted)
		AddLabel(FText::Format(NSLOCTEXT("Presentation", "WardGranted", "Ward: {0}"),
		                       StaticEnum<EDamageSource>()->GetDisplayValueAsText(Source)), WardColor);
}

#include "Presentation/Tactical/Converters/StatusChangeStepConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Units/Unit.h"
#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"

UFloatingTextPresentationAction* TacticalLogConverters::ConvertStatusChangeStep(const FTacLogStatusChangeStep& Step, const FPresentationBuildContext& Context)
{
	AUnit* const* FoundUnit = Context.UnitLookup->Find(Step.UnitId);
	checkf(FoundUnit, TEXT("Status change step references unit %s - unit is present in UnitLookup"), *Step.UnitId.ToString());

	const FLinearColor ActiveColor   = Context.Config ? Context.Config->StatusActivatedColor   : FLinearColor::Yellow;
	const FLinearColor InactiveColor = Context.Config ? Context.Config->StatusDeactivatedColor : FLinearColor::Gray;

	UFloatingTextPresentationAction* Action = NewObject<UFloatingTextPresentationAction>();
	Action->Context.Actor = *FoundUnit;
	Action->Context.Text = StaticEnum<EUnitStatus>()->GetDisplayValueAsText(Step.Status);
	Action->Context.Color = Step.bActivated ? ActiveColor : InactiveColor;
	return Action;
}

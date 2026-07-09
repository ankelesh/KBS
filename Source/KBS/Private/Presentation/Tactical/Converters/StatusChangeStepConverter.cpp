#include "Presentation/Tactical/Converters/StatusChangeStepConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Units/Unit.h"
#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"

UFloatingTextPresentationAction* TacticalLogConverters::ConvertStatusChangeStep(const FTacLogStatusChangeStep& Step, const TMap<FGuid, AUnit*>& UnitLookup)
{
	AUnit* const* FoundUnit = UnitLookup.Find(Step.UnitId);
	checkf(FoundUnit, TEXT("Status change step references unit %s not found on grid"), *Step.UnitId.ToString());

	UFloatingTextPresentationAction* Action = NewObject<UFloatingTextPresentationAction>();
	Action->Context.Actor = *FoundUnit;
	Action->Context.Text = StaticEnum<EUnitStatus>()->GetDisplayValueAsText(static_cast<int64>(Step.Status));
	Action->Context.Color = Step.bActivated ? FLinearColor::Yellow : FLinearColor::Gray;
	return Action;
}

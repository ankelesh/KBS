#include "Presentation/Tactical/Actions/TacticalMovePresentationAction.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Components/UnitVisualsComponent.h"

void UTacticalMovePresentationAction::OnMoveStarted()
{
	AUnit* Unit = Cast<AUnit>(Context.Actor);
	checkf(Unit, TEXT("UTacticalMovePresentationAction: Actor must be an AUnit"));
	Unit->GetVisualsComponent()->SetIsMoving(true);
}

void UTacticalMovePresentationAction::OnMoveFinished()
{
	if (AUnit* Unit = Cast<AUnit>(Context.Actor))
	{
		Unit->GetVisualsComponent()->SetIsMoving(false);
	}
}

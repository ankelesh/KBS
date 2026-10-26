#include "Presentation/Tactical/Converters/FleeStepConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/TacticalMovementConstants.h"
#include "Presentation/Core/Actions/MovePresentationAction.h"

UMovePresentationAction* TacticalLogConverters::ConvertFleeStep(const FTacLogFleeStep& Step, const FPresentationBuildContext& Context)
{
	AUnit* const* FoundUnit = Context.UnitLookup->Find(Step.UnitId);
	checkf(FoundUnit, TEXT("Flee step references unit %s - unit is present in UnitLookup"), *Step.UnitId.ToString());
	AUnit* Unit = *FoundUnit;

	const FVector Pos = Context.GridSubsystem->GetCellWorldLocation(Step.UnitCoords);
	const float FleeYaw = (Step.TeamSide == ETeamSide::Attacker)
		? FTacMovementConstants::DefenderDefaultYaw
		: FTacMovementConstants::AttackerDefaultYaw;

	// TODO: rotation currently snaps (zero-duration segment) since no standard turn-in-place
	// duration is defined yet - mirrors the bIsAnimated TODO in MoveStepConverter.cpp.
	UMovePresentationAction* Action = NewObject<UMovePresentationAction>();
	Action->Context.Actor = Unit;
	Action->Context.Path.Add(FPresentationMoveSegment(Pos, Pos, 0.f, FRotator(0.f, FleeYaw, 0.f)));
	return Action;
}

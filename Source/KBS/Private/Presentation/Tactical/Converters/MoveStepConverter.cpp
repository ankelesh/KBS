#include "Presentation/Tactical/Converters/MoveStepConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Units/Unit.h"
#include "Presentation/Tactical/Actions/TacticalMovePresentationAction.h"
#include "GameplayTypes/TacticalMovementConstants.h"

UMovePresentationAction* TacticalLogConverters::ConvertMoveStep(const FTacLogMoveStep& Step, const FPresentationBuildContext& Context)
{
	AUnit* const* FoundUnit = Context.UnitLookup->Find(Step.UnitId);
	checkf(FoundUnit, TEXT("Move step references unit %s - unit is present in UnitLookup"), *Step.UnitId.ToString());
	AUnit* Unit = *FoundUnit;

	const FVector Start = Context.GridSubsystem->GetCellWorldLocation(Step.FromCoords);
	const FVector End = Context.GridSubsystem->GetCellWorldLocation(Step.ToCoords);
	const float Duration = FVector::Dist(Start, End) / Unit->GetMovementSpeed();
	FRotator TargetRotation = (End - Start).Rotation();
	TargetRotation.Yaw += FTacMovementConstants::ModelForwardOffset;

	// TODO: respect Step.bIsAnimated (snap vs animate) once decided how that maps to TransitionPolicy/PlaybackMode.

	UMovePresentationAction* Action = NewObject<UTacticalMovePresentationAction>();
	Action->Context.Actor = Unit;
	Action->Context.Path.Add(FPresentationMoveSegment(Start, End, Duration, TargetRotation));
	return Action;
}

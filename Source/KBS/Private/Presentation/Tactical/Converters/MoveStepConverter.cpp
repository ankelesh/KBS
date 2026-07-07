#include "Presentation/Tactical/Converters/MoveStepConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"
#include "Presentation/Core/Actions/MovePresentationAction.h"

UMovePresentationAction* TacticalLogConverters::ConvertMoveStep(const FTacLogMoveStep& Step, const TMap<FGuid, AUnit*>& UnitLookup, UGridDataManager* GridDataManager)
{
	AUnit* const* FoundUnit = UnitLookup.Find(Step.UnitId);
	checkf(FoundUnit, TEXT("Move step references unit %s not found on grid"), *Step.UnitId.ToString());
	AUnit* Unit = *FoundUnit;

	const FVector Start = GridDataManager->GetCellWorldLocation(Step.FromCoords);
	const FVector End = GridDataManager->GetCellWorldLocation(Step.ToCoords);
	const float Duration = FVector::Dist(Start, End) / Unit->GetMovementSpeed();
	const FRotator TargetRotation = (End - Start).Rotation();

	// TODO: respect Step.bIsAnimated (snap vs animate) once decided how that maps to TransitionPolicy/PlaybackMode.

	UMovePresentationAction* Action = NewObject<UMovePresentationAction>();
	Action->Context.Actor = Unit;
	Action->Context.Path.Add(FPresentationMoveSegment(Start, End, Duration, TargetRotation));
	return Action;
}

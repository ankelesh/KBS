#include "Presentation/Tactical/Actions/DespawnPresentationAction.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"

void UDespawnPresentationAction::OnExecute(EPlaybackMode PlaybackMode)
{
	checkf(Context.Unit, TEXT("UDespawnPresentationAction: Unit must not be null"));

	FinishExecution(EVisualActionResult::Completed);
}

void UDespawnPresentationAction::OnCleanup()
{
	// Presentation for this unit's despawn has fully played out - safe to finalize now.
	Context.Unit->GetWorld()->GetSubsystem<UTacGridSubsystem>()->FinalizeDespawnedUnit(Context.Unit);
	FinishCleanup();
}

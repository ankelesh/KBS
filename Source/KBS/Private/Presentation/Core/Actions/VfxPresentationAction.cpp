#include "Presentation/Core/Actions/VfxPresentationAction.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

void UVfxPresentationAction::OnExecute(EPlaybackMode PlaybackMode)
{
	checkf(Context.Actor, TEXT("UVfxPresentationAction: Actor must not be null"));
	checkf(Context.System, TEXT("UVfxPresentationAction: System must not be null"));

	// Fire-and-forget: Niagara self-destroys; accompanying combat/step actions gate the sequence.
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		Context.Actor->GetWorld(),
		Context.System,
		Context.Actor->GetActorLocation() + FVector(0, 0, Context.ZOffset),
		FRotator::ZeroRotator, FVector::OneVector,
		/*bAutoDestroy*/ true, /*bAutoActivate*/ true,
		ENCPoolMethod::None, /*bPreCullCheck*/ true);

	FinishExecution(EVisualActionResult::Completed);
}

void UVfxPresentationAction::OnCleanup()
{
	FinishCleanup();
}

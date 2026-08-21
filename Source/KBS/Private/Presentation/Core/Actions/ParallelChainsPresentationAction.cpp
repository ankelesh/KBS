#include "Presentation/Core/Actions/ParallelChainsPresentationAction.h"

void UParallelChainsPresentationAction::InitRemainingChains(int32 Count)
{
	RemainingChains = Count;
	if (Count == 0)
	{
		FinishExecution(EVisualActionResult::Completed);
	}
}

void UParallelChainsPresentationAction::NotifyChainFinished()
{
	if (--RemainingChains <= 0)
	{
		FinishExecution(EVisualActionResult::Completed);
	}
}

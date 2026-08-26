#include "Presentation/Tactical/Actions/AoEReactionPresentationAction.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Components/UnitVisualsComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogAoEReaction, Log, All);

void UAoEReactionPresentationAction::OnExecute(EPlaybackMode PlaybackMode)
{
	CurrentPlaybackMode = PlaybackMode;
	ChainCursors.Init(0, Chains.Num());

	int32 ActiveChains = 0;
	for (const FAoEReactionChain& Chain : Chains)
	{
		if (!Chain.Steps.IsEmpty())
		{
			++ActiveChains;
		}
	}

	InitRemainingChains(ActiveChains);
	if (ActiveChains == 0)
	{
		return;
	}

	for (int32 ChainIndex = 0; ChainIndex < Chains.Num(); ++ChainIndex)
	{
		StartChainStep(ChainIndex, 0);
	}
}

void UAoEReactionPresentationAction::StartChainStep(int32 ChainIndex, int32 StepIndex)
{
	const TArray<FAoEReactionStep>& Steps = Chains[ChainIndex].Steps;
	if (!Steps.IsValidIndex(StepIndex))
	{
		NotifyChainFinished();
		return;
	}

	ChainCursors[ChainIndex] = StepIndex;
	const FAoEReactionStep& Step = Steps[StepIndex];

	// Marked before Play(): the Slot is now downstream of the Dead-pose branch (Dead pose is the base
	// the Slot layers on top of), so the montage still plays in full regardless of bIsDead - flipping
	// it early just means the base revealed during this step's own blend-in/out is already correct,
	// instead of leaving a window where it's still "alive".
	if (Step.bMarksActorDead)
	{
		AUnit* Unit = Cast<AUnit>(Step.Context.Actor);
		checkf(Unit, TEXT("AoEReactionPresentationAction: bMarksActorDead step's actor must be an AUnit"));
		UE_LOG(LogAoEReaction, Log, TEXT("Marking %s dead (step starting)"), *Unit->GetName());
		Unit->GetVisualsComponent()->SetIsDead(true);
	}

	UMontagePlaybackHelper* Helper = NewObject<UMontagePlaybackHelper>(this);
	ActiveHelpers.Add(Helper);

	Helper->OnFinished.BindLambda([this, ChainIndex](EVisualActionResult StepResult)
	{
		UE_LOG(LogAoEReaction, Log, TEXT("Chain %d step %d finished with result %d"),
			ChainIndex, ChainCursors[ChainIndex], (int32)StepResult);
		StartChainStep(ChainIndex, ChainCursors[ChainIndex] + 1);
	});

	if (Step.TimeoutDuration > 0.f)
	{
		UWorld* World = Step.Context.Actor ? Step.Context.Actor->GetWorld() : nullptr;
		Helper->StartTimeout(World, Step.TimeoutDuration);
	}
	Helper->Play(Step.Context, CurrentPlaybackMode);
}

void UAoEReactionPresentationAction::OnCleanup()
{
	if (ActiveHelpers.IsEmpty())
	{
		FinishCleanup();
		return;
	}

	PendingCleanupCount = ActiveHelpers.Num();
	for (UMontagePlaybackHelper* Helper : ActiveHelpers)
	{
		Helper->OnCleanupFinished.BindLambda([this]()
		{
			if (--PendingCleanupCount == 0)
			{
				FinishCleanup();
			}
		});
		Helper->Cleanup();
	}
}

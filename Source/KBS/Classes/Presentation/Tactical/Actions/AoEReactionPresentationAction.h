#pragma once
#include "CoreMinimal.h"
#include "Presentation/Core/PresentationSequenceAction.h"
#include "Presentation/Core/Actions/MontagePlaybackHelper.h"
#include "AoEReactionPresentationAction.generated.h"

// A single montage step within a target's chain, with its own optional forced-advance timeout
// (e.g. death montages that pose-hold and never fire OnMontageEnded naturally).
USTRUCT(BlueprintType)
struct FAoEReactionStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	FMontageActionContext Context;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	float TimeoutDuration = 0.f;

	// Set on the actor's UnitVisualsComponent once this step finishes playing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	bool bMarksActorDead = false;
};

// One target's ordered reaction chain (e.g. hit-reaction then death).
USTRUCT(BlueprintType)
struct FAoEReactionChain
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	TArray<FAoEReactionStep> Steps;
};

// Plays every target's reaction chain at the same time - one hit reaction/death chain per target,
// running concurrently with the others - and finishes once all chains are done. Built directly on
// UMontagePlaybackHelper (not on other UPresentationSequenceActions): the sequence player is the
// only thing that gets to coordinate action lifecycles, so this action stays a single leaf in the
// sequence rather than nesting actions inside itself.
UCLASS(BlueprintType, Blueprintable)
class KBS_API UAoEReactionPresentationAction : public UPresentationSequenceAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	TArray<FAoEReactionChain> Chains;

protected:
	virtual void OnExecute(EPlaybackMode PlaybackMode) override;
	virtual void OnCleanup() override;

private:
	void StartChainStep(int32 ChainIndex, int32 StepIndex);

	UPROPERTY()
	TArray<TObjectPtr<UMontagePlaybackHelper>> ActiveHelpers;

	TArray<int32> ChainCursors;
	EPlaybackMode CurrentPlaybackMode = EPlaybackMode::Animated;
	int32 RemainingChains = 0;
	int32 PendingCleanupCount = 0;
};

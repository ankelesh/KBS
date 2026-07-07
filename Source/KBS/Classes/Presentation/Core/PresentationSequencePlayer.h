#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PresentationSequencePlayer.generated.h"

class UPresentationSequence;

UENUM(BlueprintType)
enum class EPresentationPlayerState : uint8
{
	Idle,
	Playing,
};

UENUM(BlueprintType)
enum class EPlaybackMode : uint8
{
	Animated,
	Instant,
};

// Note: OnPresentationComplete() method exists on TacTurnSubsystem/TacTurnState — no type collision, just be aware.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPresentationComplete);

UCLASS()
class KBS_API UPresentationSequencePlayer : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static UPresentationSequencePlayer* Get(const UObject* WorldContextObject);

	// Contract: only while Idle
	UFUNCTION(BlueprintCallable, Category = "Presentation")
	void EnqueueSequence(UPresentationSequence* Sequence);

	// Plays the whole queue; contract: only while Idle
	UFUNCTION(BlueprintCallable, Category = "Presentation")
	void Play(EPlaybackMode PlaybackMode = EPlaybackMode::Animated);

	UFUNCTION(BlueprintCallable, Category = "Presentation")
	EPresentationPlayerState GetState() const { return State; }

	UPROPERTY(BlueprintAssignable, Category = "Presentation")
	FOnPresentationComplete OnPresentationComplete;

private:
	UFUNCTION()
	void HandleSequenceComplete(UPresentationSequence* Sequence);
	void PlayNext();

	UPROPERTY()
	TArray<TObjectPtr<UPresentationSequence>> Queue;
	int32 QueueIndex = 0;
	EPlaybackMode CurrentPlaybackMode = EPlaybackMode::Animated;
	EPresentationPlayerState State = EPresentationPlayerState::Idle;
};

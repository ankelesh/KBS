#pragma once
#include "CoreMinimal.h"
#include "Presentation/Core/Actions/ParallelChainsPresentationAction.h"
#include "Presentation/Core/Actions/ProjectilePresentationAction.h"
#include "DeliveryPresentationAction.generated.h"

class UNiagaraSystem;
class AProjectilePresentationActor;

USTRUCT()
struct FDeliveryVfxChain
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ImpactVFX;

	// Provides the world for spawning; must not be null.
	UPROPERTY()
	TObjectPtr<AActor> WorldContextActor;
};

// Plays the delivery phase of a combat step: one projectile or VFX chain per hit target,
// running in parallel. Start delays are pre-seeded from EventId + target index.
// Created by ConvertCombatStep; topology and data come from FDeliveryPresentation.
UCLASS(BlueprintType, Blueprintable)
class KBS_API UDeliveryPresentationAction : public UParallelChainsPresentationAction
{
	GENERATED_BODY()

public:
	// Projectile chains (OnePerTarget, SingleToPoint topologies).
	UPROPERTY()
	TArray<FProjectileActionContext> ProjectileContexts;

	// VFX-only chains (PerTargetVfx topology).
	UPROPERTY()
	TArray<FDeliveryVfxChain> VfxChains;

	// Per-chain start delay in seconds, indexed by chain position.
	TArray<float> ChainStartDelays;

protected:
	virtual void OnExecute(EPlaybackMode PlaybackMode) override;
	virtual void OnCleanup() override;

private:
	void ExecuteProjectileChain(int32 ChainIndex, EPlaybackMode PlaybackMode);
	void ExecuteVfxChain(int32 ChainIndex);

	UPROPERTY()
	TArray<TObjectPtr<AProjectilePresentationActor>> SpawnedProjectiles;

	TArray<FTimerHandle> StartDelayTimers;
	int32 PendingCleanupCount = 0;
};

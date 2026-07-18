#pragma once
#include "CoreMinimal.h"
#include "Presentation/Core/PresentationSequenceAction.h"
#include "Presentation/Core/Actions/ProjectilePresentationActor.h"
#include "ProjectilePresentationAction.generated.h"

class UNiagaraSystem;
class USoundBase;

USTRUCT(BlueprintType)
struct FProjectileActionContext
{
	GENERATED_BODY()

	// Any actor in the world providing GetWorld() for spawning (e.g. the attacker/caster).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<AActor> WorldContextActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FVector EndLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "1.0"))
	float Speed = 1000.f;

	// Optional: the projectile's own travelling visual/sound (arrow/orb/trail hum).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Effects")
	TObjectPtr<UNiagaraSystem> TrailVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Effects")
	TObjectPtr<USoundBase> TrailSFX;

	// Optional; if set, its completion gates the action's finish. If unset, action finishes as
	// soon as the projectile reaches EndLocation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Effects")
	TObjectPtr<UNiagaraSystem> ImpactVFX;

	// Optional, fire-and-forget - never gates completion.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Effects")
	TObjectPtr<USoundBase> ImpactSFX;

	// Optional BP override (different meshes/components per spell); defaults to the base class.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<AProjectilePresentationActor> ActorClass;
};

// Spawns a travelling projectile actor and finishes once it reaches its destination and any
// impact VFX has played out. The projectile actor is despawned in Cleanup(), not on finish.
UCLASS(BlueprintType, Blueprintable)
class KBS_API UProjectilePresentationAction : public UPresentationSequenceAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FProjectileActionContext Context;

protected:
	virtual void OnExecute(EPlaybackMode PlaybackMode) override;
	virtual void OnCleanup() override;

private:
	void HandleProjectileFinished();

	UPROPERTY()
	TObjectPtr<AProjectilePresentationActor> SpawnedProjectile;
};

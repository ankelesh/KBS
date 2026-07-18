#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Ticker.h"
#include "ProjectilePresentationActor.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;
class UAudioComponent;

// Self-contained travelling projectile visual (arrow/spell orb). Owns movement + trail/impact
// VFX/SFX mechanics; UProjectilePresentationAction owns the sequencer contract around it.
// Does NOT self-destroy - the owning action destroys it in its Cleanup() step.
UCLASS(BlueprintType, Blueprintable)
class KBS_API AProjectilePresentationActor : public AActor
{
	GENERATED_BODY()

public:
	AProjectilePresentationActor();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	DECLARE_DELEGATE(FOnProjectileFinished);
	// Fires once travel reaches End AND ImpactVFX (if any) finishes playing.
	FOnProjectileFinished OnFinished;

	// TrailVFX/TrailSFX are the projectile's own travelling visuals/sound. ImpactVFX/ImpactSFX
	// play at End on arrival. ImpactVFX (if set) gates OnFinished; ImpactSFX is fire-and-forget
	// and never gates completion. All four may be null.
	void Launch(const FVector& Start, const FVector& End, float Speed,
	            UNiagaraSystem* TrailVFX, USoundBase* TrailSFX,
	            UNiagaraSystem* ImpactVFX, USoundBase* ImpactSFX);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USceneComponent> Root;

private:
	bool OnTick(float DeltaTime);
	void PlayImpact();
	UFUNCTION()
	void HandleImpactVFXFinished(UNiagaraComponent* FinishedComponent);

	FTSTicker::FDelegateHandle TickerHandle;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> TrailVFXComponent;
	UPROPERTY()
	TObjectPtr<UAudioComponent> TrailSFXComponent;

	FVector StartLocation = FVector::ZeroVector;
	FVector EndLocation = FVector::ZeroVector;
	float Duration = 0.f;
	float Elapsed = 0.f;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> PendingImpactVFX;
	UPROPERTY()
	TObjectPtr<USoundBase> PendingImpactSFX;
};

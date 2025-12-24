#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameMechanics/Tactical/Grid/Components/PresentationTrackerComponent.h"
#include "UnitVisualsComponent.generated.h"
class UUnitDefinition;
class USkeletalMeshComponent;
class UStaticMesh;
class UAnimMontage;
class UNiagaraSystem;
class UNiagaraComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMontageCompleted, UAnimMontage*, Montage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRotationCompleted);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMontageCompletedNative, UAnimMontage*);
DECLARE_MULTICAST_DELEGATE(FOnRotationCompletedNative);
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KBS_API UUnitVisualsComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	UUnitVisualsComponent();
	void InitializeFromDefinition(UUnitDefinition* Definition);
	void ClearAllMeshComponents();
	void AttachWeaponMesh(UStaticMesh* WeaponMesh, FName SocketName);
	void DetachWeaponMesh(UMeshComponent* WeaponMeshComponent);
	USkeletalMeshComponent* GetPrimarySkeletalMesh() const { return PrimarySkeletalMesh; }
	const TArray<TObjectPtr<USceneComponent>>& GetAllMeshComponents() const { return SpawnedMeshComponents; }
	void PlayAttackMontage(UAnimMontage* Montage, float PlayRate = 1.0f);
	void PlayHitReactionMontage(UAnimMontage* Montage);
	void PlayDeathMontage(UAnimMontage* Montage);
	void StopAllMontages();
	void SetMovementSpeed(float Speed);
	void SetIsMoving(bool bMoving);
	void RotateTowardTarget(FRotator TargetRotation, float Speed = 360.0f);
	bool IsRotating() const { return bIsRotating; }
	UNiagaraComponent* SpawnNiagaraEffect(UNiagaraSystem* System, FVector WorldLocation, float Duration);
	void SetPresentationTracker(class UPresentationTrackerComponent* InTracker);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY(BlueprintAssignable, Category = "Animation")
	FOnMontageCompleted OnMontageCompleted;
	UPROPERTY(BlueprintAssignable, Category = "Animation")
	FOnRotationCompleted OnRotationCompleted;
	FOnMontageCompletedNative OnMontageCompletedNative;
	FOnRotationCompletedNative OnRotationCompletedNative;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> VisualsRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> PrimarySkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<TObjectPtr<USceneComponent>> SpawnedMeshComponents;
private:
	void CreateMeshComponent(const struct FUnitMeshDescriptor& Descriptor, UUnitDefinition* Definition);
	void SetupCollisionForMesh(UPrimitiveComponent* MeshComponent);
	void HandleMontageCompleted(UAnimMontage* Montage, bool bInterrupted);
	FRotator PendingRotation;
	bool bIsRotating = false;
	float CurrentRotationSpeed = 360.0f;
	UPROPERTY()
	TObjectPtr<class UPresentationTrackerComponent> PresentationTracker;
	struct FVFXTrackingData
	{
		FTimerHandle TimerHandle;
		FOperationHandle OperationHandle;
	};
	TMap<TObjectPtr<UNiagaraComponent>, FVFXTrackingData> ActiveVFXOperations;
	void OnVFXCompleted(UNiagaraComponent* Component);
};

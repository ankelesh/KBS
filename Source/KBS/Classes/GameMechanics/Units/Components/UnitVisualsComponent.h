#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTagContainer.h"
#include "UnitVisualsComponent.generated.h"
class UUnitVisualDefinition;
class UUnitAnimationSet;
class USkeletalMeshComponent;
class UStaticMesh;
class UAnimMontage;
class UBattleEffect;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KBS_API UUnitVisualsComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	UUnitVisualsComponent();
	virtual void BeginPlay() override;
	void InitializeFromDefinition(UUnitVisualDefinition* Definition);
	void SwapVisualDefinition(UUnitVisualDefinition* NewDefinition);
	void SetUnitSize(int32 InUnitSize) { CachedUnitSize = InUnitSize; }
	void ClearAllMeshComponents();
	void AttachWeaponMesh(UStaticMesh* WeaponMesh, FName SocketName);
	void DetachWeaponMesh(UMeshComponent* WeaponMeshComponent);
	USkeletalMeshComponent* GetPrimarySkeletalMesh() const { return PrimarySkeletalMesh; }
	const TArray<TObjectPtr<USceneComponent>>& GetAllMeshComponents() const { return SpawnedMeshComponents; }
	void SetCellSize(float InCellSize) { CachedCellSize = InCellSize; }
	void ReverseExtraCellOffset();
	// Resolves a montage by tag with parent-tag fallback (e.g. Animation.Attack.Slash -> Animation.Attack)
	UAnimMontage* ResolveAnimation(FGameplayTag Tag) const;
	void SetIsDead(bool bDead);
	void SetIsMoving(bool bMoving);
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> VisualsRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> PrimarySkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<TObjectPtr<USceneComponent>> SpawnedMeshComponents;
private:
	void CreateMeshComponent(const struct FUnitMeshDescriptor& Descriptor, UUnitVisualDefinition* Definition);
	void SetupCollisionForMesh(UPrimitiveComponent* MeshComponent);
	UFUNCTION() void OnOwnerFieldPresenceChanged(AUnit* Unit, bool bIsOnField);
	void OnOwnerOrientationChanged(EUnitOrientation NewOrientation);
	static FRotator OrientationToRotation(EUnitOrientation Orientation);

	TObjectPtr<UUnitAnimationSet> AnimationSet;

	int32 CachedUnitSize = 1;
	float CachedCellSize = 200.0f;
};

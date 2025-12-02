#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "UnitVisualsComponent.generated.h"

class UUnitDefinition;
class USkeletalMeshComponent;
class UStaticMesh;

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
};

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UnitVisualDefinition.generated.h"

class USkeletalMesh;
class UStaticMesh;
class UMaterialInterface;
class UAnimMontage;

UENUM(BlueprintType)
enum class EUnitMeshType : uint8
{
	Skeletal UMETA(DisplayName = "Skeletal Mesh"),
	Static   UMETA(DisplayName = "Static Mesh")
};

USTRUCT(BlueprintType)
struct FUnitMeshDescriptor
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	EUnitMeshType MeshType = EUnitMeshType::Skeletal;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (EditCondition = "MeshType == EUnitMeshType::Skeletal", EditConditionHides))
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (EditCondition = "MeshType == EUnitMeshType::Static", EditConditionHides))
	TSoftObjectPtr<UStaticMesh> StaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachment")
	FName ParentSocket = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachment")
	FTransform RelativeTransform = FTransform::Identity;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
	TArray<TSoftObjectPtr<UMaterialInterface>> MaterialOverrides;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsPrimaryMesh = false;
};

UCLASS(BlueprintType)
class KBS_API UUnitAnimationSet : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// Tag-to-montage map. Lookup falls back to parent tags (e.g. Attack.Slash -> Attack).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> Montages;
};

// Visual-only data asset for a unit. Can be swapped at runtime for mid-game appearance changes.
// Montages (death, hit reaction, attack) are resolved by tag from AnimationSet — see UnitVisualTags.h.
UCLASS(BlueprintType)
class KBS_API UUnitVisualDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	TArray<FUnitMeshDescriptor> MeshComponents;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> AnimationClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	TObjectPtr<UTexture2D> Portrait;
	// All montages are stored here, keyed by gameplay tag (e.g. Animation.Death, Animation.Attack).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UUnitAnimationSet> AnimationSet;
};

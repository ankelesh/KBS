#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameMechanics/Units/Stats/UnitStats.h"
#include "UnitDefinition.generated.h"
class UWeaponDataAsset;
class USkeletalMesh;
class UStaticMesh;
class UAnimBlueprintGeneratedClass;
class UMaterialInterface;
class UAnimMontage;
class UUnitAbilityDefinition;
constexpr int32 NoWeaponDamageOverride = -1;

USTRUCT(BlueprintType)
struct FUnitWeaponEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UWeaponDataAsset> Weapon;

	// NoWeaponDamageOverride = use weapon asset's own BaseDamage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 BaseDamageOverride = NoWeaponDamageOverride;
};

UENUM(BlueprintType)
enum class EUnitMeshType : uint8
{
	Skeletal UMETA(DisplayName = "Skeletal Mesh"),
	Static UMETA(DisplayName = "Static Mesh")
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
class KBS_API UUnitDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TArray<FUnitMeshDescriptor> MeshComponents;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSubclassOf<UAnimInstance> AnimationClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UTexture2D> Portrait;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FString UnitName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FUnitCoreStats BaseStatsTemplate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<FUnitWeaponEntry> DefaultWeapons;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots", meta = (DisplayName = "Attack"))
	TObjectPtr<UUnitAbilityDefinition> DefaultAttackAbility;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots", meta = (DisplayName = "Move"))
	TObjectPtr<UUnitAbilityDefinition> DefaultMoveAbility;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots", meta = (DisplayName = "Wait"))
	TObjectPtr<UUnitAbilityDefinition> DefaultWaitAbility;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots", meta = (DisplayName = "Defend"))
	TObjectPtr<UUnitAbilityDefinition> DefaultDefendAbility;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots", meta = (DisplayName = "Flee"))
	TObjectPtr<UUnitAbilityDefinition> DefaultFleeAbility;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UUnitAbilityDefinition>> AdditionalAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|Spellbook")
	TArray<TObjectPtr<UUnitAbilityDefinition>> SpellbookAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DeathMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitReactionMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MovementSpeed = 300.0f;
};

#include "GameMechanics/Units/Components/UnitVisualsComponent.h"
#include "GameMechanics/Units/Components/Config/UnitVisualDefinition.h"
#include "GameMechanics/Units/Components/UnitAnimInstance.h"
#include "GameMechanics/Units/Unit.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
UUnitVisualsComponent::UUnitVisualsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetMobility(EComponentMobility::Movable);
}

void UUnitVisualsComponent::BeginPlay()
{
	Super::BeginPlay();
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	checkf(OwnerUnit, TEXT("UUnitVisualsComponent must be owned by AUnit"));
	OwnerUnit->OnOrientationChanged.AddUObject(this, &UUnitVisualsComponent::OnOwnerOrientationChanged);
	OwnerUnit->OnUnitFieldPresenceChange.AddDynamic(this, &UUnitVisualsComponent::OnOwnerFieldPresenceChanged);
}

void UUnitVisualsComponent::OnOwnerOrientationChanged(EUnitOrientation NewOrientation)
{
	GetOwner()->SetActorRotation(OrientationToRotation(NewOrientation));
	if (VisualsRoot)
	{
		// For 2-cell units the model center sits between primary and extra cell.
		// Extra cell is always at local (0, -CellSize, 0), so shift by half that.
		const FVector Offset = CachedUnitSize > 1
			? FVector(0.0f, -CachedCellSize * 0.5f, 0.0f)
			: FVector::ZeroVector;
		VisualsRoot->SetRelativeLocation(Offset);
	}
}

FRotator UUnitVisualsComponent::OrientationToRotation(EUnitOrientation Orientation)
{
	switch (Orientation)
	{
	case EUnitOrientation::GridBottom: return FRotator(0.0f,   0.0f, 0.0f);
	case EUnitOrientation::GridTop:    return FRotator(0.0f, 180.0f, 0.0f);
	case EUnitOrientation::GridRight:  return FRotator(0.0f, -90.0f, 0.0f);
	case EUnitOrientation::GridLeft:   return FRotator(0.0f,  90.0f, 0.0f);
	}
	return FRotator::ZeroRotator;
}

void UUnitVisualsComponent::ReverseExtraCellOffset()
{
	if (VisualsRoot && CachedUnitSize > 1)
		VisualsRoot->SetRelativeLocation(FVector(0.f, CachedCellSize * 0.5f, 0.f));
}

void UUnitVisualsComponent::OnOwnerFieldPresenceChanged(AUnit* Unit, bool bIsOnField)
{
	VisualsRoot->SetVisibility(bIsOnField, true);
}

void UUnitVisualsComponent::ClearAllMeshComponents()
{
	for (int32 i = SpawnedMeshComponents.Num() - 1; i >= 0; --i)
	{
		if (SpawnedMeshComponents[i])
		{
			SpawnedMeshComponents[i]->DestroyComponent();
		}
	}
	SpawnedMeshComponents.Empty();
	PrimarySkeletalMesh = nullptr;
}
void UUnitVisualsComponent::SwapVisualDefinition(UUnitVisualDefinition* NewDefinition)
{
	ClearAllMeshComponents();
	InitializeFromDefinition(NewDefinition);
}

void UUnitVisualsComponent::InitializeFromDefinition(UUnitVisualDefinition* Definition)
{
	if (!Definition)
	{
		UE_LOG(LogTemp, Error, TEXT("UUnitVisualsComponent: Cannot initialize from null UnitVisualDefinition"));
		return;
	}
	AnimationSet = Definition->AnimationSet;
	if (Definition->MeshComponents.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UUnitVisualsComponent: No mesh components defined in UnitVisualDefinition"));
		return;
	}
	if (!VisualsRoot)
	{
		VisualsRoot = NewObject<USceneComponent>(GetOwner(), USceneComponent::StaticClass(), TEXT("VisualsRoot"));
		if (VisualsRoot)
		{
			VisualsRoot->SetupAttachment(this);
			VisualsRoot->SetMobility(EComponentMobility::Movable);
			VisualsRoot->RegisterComponent();
		}
	}
	for (const FUnitMeshDescriptor& MeshDesc : Definition->MeshComponents)
	{
		if (MeshDesc.MeshType == EUnitMeshType::Skeletal)
		{
			CreateMeshComponent(MeshDesc, Definition);
		}
	}
	if (PrimarySkeletalMesh && Definition->AnimationClass)
	{
		PrimarySkeletalMesh->SetAnimInstanceClass(Definition->AnimationClass);
		PrimarySkeletalMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		PrimarySkeletalMesh->InitAnim(true);
	}
	if (PrimarySkeletalMesh)
	{
		for (USceneComponent* Component : SpawnedMeshComponents)
		{
			USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(Component);
			if (SkelMesh && SkelMesh != PrimarySkeletalMesh)
			{
				SkelMesh->SetLeaderPoseComponent(PrimarySkeletalMesh);
#if WITH_EDITOR
				SkelMesh->TickAnimation(0.0f, false);
#endif
			}
		}
	}
	for (const FUnitMeshDescriptor& MeshDesc : Definition->MeshComponents)
	{
		if (MeshDesc.MeshType == EUnitMeshType::Static)
		{
			CreateMeshComponent(MeshDesc, Definition);
		}
	}
}
void UUnitVisualsComponent::CreateMeshComponent(const FUnitMeshDescriptor& Descriptor, UUnitVisualDefinition* Definition)
{
	USceneComponent* NewMeshComponent = nullptr;
	UPrimitiveComponent* PrimitiveComp = nullptr;
	if (Descriptor.MeshType == EUnitMeshType::Skeletal)
	{
		if (!Descriptor.SkeletalMesh.IsNull())
		{
			USkeletalMeshComponent* SkelMeshComp = NewObject<USkeletalMeshComponent>(
				GetOwner(),
				USkeletalMeshComponent::StaticClass(),
				MakeUniqueObjectName(GetOwner(), USkeletalMeshComponent::StaticClass(), TEXT("SkeletalMesh"))
			);
			if (SkelMeshComp)
			{
				USkeletalMesh* LoadedMesh = Descriptor.SkeletalMesh.LoadSynchronous();
				if (LoadedMesh)
				{
					SkelMeshComp->SetSkeletalMesh(LoadedMesh);
					SkelMeshComp->SetComponentTickEnabled(true);
					SkelMeshComp->PrimaryComponentTick.bCanEverTick = true;
					SkelMeshComp->SetMobility(EComponentMobility::Movable);
#if WITH_EDITOR
					SkelMeshComp->SetUpdateAnimationInEditor(true);
#endif
					NewMeshComponent = SkelMeshComp;
					PrimitiveComp = SkelMeshComp;
					if (Descriptor.bIsPrimaryMesh && !PrimarySkeletalMesh)
					{
						PrimarySkeletalMesh = SkelMeshComp;
					}
				}
			}
		}
	}
	else if (Descriptor.MeshType == EUnitMeshType::Static)
	{
		if (!Descriptor.StaticMesh.IsNull())
		{
			UStaticMeshComponent* StaticMeshComp = NewObject<UStaticMeshComponent>(
				GetOwner(),
				UStaticMeshComponent::StaticClass(),
				MakeUniqueObjectName(GetOwner(), UStaticMeshComponent::StaticClass(), TEXT("StaticMesh"))
			);
			if (StaticMeshComp)
			{
				UStaticMesh* LoadedMesh = Descriptor.StaticMesh.LoadSynchronous();
				if (LoadedMesh)
				{
					StaticMeshComp->SetStaticMesh(LoadedMesh);
					StaticMeshComp->SetMobility(EComponentMobility::Movable);
					NewMeshComponent = StaticMeshComp;
					PrimitiveComp = StaticMeshComp;
				}
			}
		}
	}
	if (!NewMeshComponent)
	{
		return;
	}
	if (Descriptor.ParentSocket != NAME_None && PrimarySkeletalMesh)
	{
		FAttachmentTransformRules AttachRules = Descriptor.RelativeTransform.Equals(FTransform::Identity)
			? FAttachmentTransformRules::SnapToTargetIncludingScale
			: FAttachmentTransformRules::KeepRelativeTransform;
		NewMeshComponent->AttachToComponent(PrimarySkeletalMesh, AttachRules, Descriptor.ParentSocket);
		if (!Descriptor.RelativeTransform.Equals(FTransform::Identity))
		{
			NewMeshComponent->SetRelativeTransform(Descriptor.RelativeTransform);
		}
	}
	else
	{
		NewMeshComponent->AttachToComponent(VisualsRoot, FAttachmentTransformRules::KeepRelativeTransform);
		NewMeshComponent->SetRelativeTransform(Descriptor.RelativeTransform);
	}
	NewMeshComponent->RegisterComponent();
	SpawnedMeshComponents.Add(NewMeshComponent);
	if (PrimitiveComp)
	{
		SetupCollisionForMesh(PrimitiveComp);
		for (int32 i = 0; i < Descriptor.MaterialOverrides.Num(); ++i)
		{
			if (!Descriptor.MaterialOverrides[i].IsNull())
			{
				UMaterialInterface* Material = Descriptor.MaterialOverrides[i].LoadSynchronous();
				if (Material)
				{
					PrimitiveComp->SetMaterial(i, Material);
				}
			}
		}
	}
}
void UUnitVisualsComponent::SetupCollisionForMesh(UPrimitiveComponent* MeshComponent)
{
	if (MeshComponent)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
}
void UUnitVisualsComponent::AttachWeaponMesh(UStaticMesh* WeaponMesh, FName SocketName)
{
	if (!WeaponMesh || !PrimarySkeletalMesh)
	{
		return;
	}
	UStaticMeshComponent* WeaponMeshComp = NewObject<UStaticMeshComponent>(
		GetOwner(),
		UStaticMeshComponent::StaticClass(),
		MakeUniqueObjectName(GetOwner(), UStaticMeshComponent::StaticClass(), TEXT("WeaponMesh"))
	);
	if (WeaponMeshComp)
	{
		WeaponMeshComp->SetStaticMesh(WeaponMesh);
		WeaponMeshComp->SetMobility(EComponentMobility::Movable);
		WeaponMeshComp->RegisterComponent();
		WeaponMeshComp->AttachToComponent(PrimarySkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
		SetupCollisionForMesh(WeaponMeshComp);
		SpawnedMeshComponents.Add(WeaponMeshComp);
	}
}
void UUnitVisualsComponent::DetachWeaponMesh(UMeshComponent* WeaponMeshComponent)
{
	if (WeaponMeshComponent)
	{
		SpawnedMeshComponents.Remove(WeaponMeshComponent);
		WeaponMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		WeaponMeshComponent->DestroyComponent();
	}
}

UAnimMontage* UUnitVisualsComponent::ResolveAnimation(FGameplayTag Tag) const
{
	if (!AnimationSet || !Tag.IsValid()) return nullptr;
	FGameplayTag Current = Tag;
	while (Current.IsValid())
	{
		if (TObjectPtr<UAnimMontage>* Found = AnimationSet->Montages.Find(Current))
			return Found->Get();
		Current = Current.RequestDirectParent();
	}
	return nullptr;
}

void UUnitVisualsComponent::SetIsDead(bool bDead)
{
	UUnitAnimInstance* AnimInstance = Cast<UUnitAnimInstance>(PrimarySkeletalMesh->GetAnimInstance());
	checkf(AnimInstance, TEXT("UUnitVisualsComponent::SetIsDead: unit's AnimBP must be parented to UUnitAnimInstance"));
	AnimInstance->SetIsDead(bDead);
}

void UUnitVisualsComponent::SetIsMoving(bool bMoving)
{
	UUnitAnimInstance* AnimInstance = Cast<UUnitAnimInstance>(PrimarySkeletalMesh->GetAnimInstance());
	checkf(AnimInstance, TEXT("UUnitVisualsComponent::SetIsMoving: unit's AnimBP must be parented to UUnitAnimInstance"));
	AnimInstance->SetIsMoving(bMoving);
}

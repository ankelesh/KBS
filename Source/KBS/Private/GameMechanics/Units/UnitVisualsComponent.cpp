#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

UUnitVisualsComponent::UUnitVisualsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetMobility(EComponentMobility::Movable);
}

void UUnitVisualsComponent::ClearAllMeshComponents()
{
	UE_LOG(LogTemp, Warning, TEXT("UUnitVisualsComponent::ClearAllMeshComponents - Clearing %d components"), SpawnedMeshComponents.Num());

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

void UUnitVisualsComponent::InitializeFromDefinition(UUnitDefinition* Definition)
{
	if (!Definition)
	{
		UE_LOG(LogTemp, Error, TEXT("UUnitVisualsComponent: Cannot initialize from null UnitDefinition"));
		return;
	}

	if (Definition->MeshComponents.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UUnitVisualsComponent: No mesh components defined in UnitDefinition"));
		return;
	}

	// Create VisualsRoot at runtime if it doesn't exist
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

	UE_LOG(LogTemp, Warning, TEXT("UUnitVisualsComponent: Owner=%s, VisualsRoot=%s, This IsRegistered=%s, MeshComponents Count=%d"),
		GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"),
		VisualsRoot ? TEXT("Valid") : TEXT("NULL"),
		IsRegistered() ? TEXT("YES") : TEXT("NO"),
		Definition->MeshComponents.Num());

	// First pass: Create all skeletal meshes to ensure primary mesh and sockets exist
	for (const FUnitMeshDescriptor& MeshDesc : Definition->MeshComponents)
	{
		if (MeshDesc.MeshType == EUnitMeshType::Skeletal)
		{
			CreateMeshComponent(MeshDesc, Definition);
		}
	}

	// Initialize animation on primary skeletal mesh
	if (PrimarySkeletalMesh && Definition->AnimationClass)
	{
		UE_LOG(LogTemp, Log, TEXT("UUnitVisualsComponent: Setting animation class on primary skeletal mesh"));
		PrimarySkeletalMesh->SetAnimInstanceClass(Definition->AnimationClass);
		PrimarySkeletalMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		PrimarySkeletalMesh->InitAnim(true);
		UE_LOG(LogTemp, Log, TEXT("UUnitVisualsComponent: Animation initialized, AnimInstance: %s"),
			PrimarySkeletalMesh->GetAnimInstance() ? TEXT("Valid") : TEXT("NULL"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UUnitVisualsComponent: Cannot init animation - PrimaryMesh: %s, AnimClass: %s"),
			PrimarySkeletalMesh ? TEXT("Valid") : TEXT("NULL"),
			Definition->AnimationClass ? TEXT("Valid") : TEXT("NULL"));
	}

	UE_LOG(LogTemp, Warning, TEXT("UUnitVisualsComponent::InitializeFromDefinition COMPLETE - SpawnedMeshComponents: %d, PrimarySkeletalMesh: %s"),
		SpawnedMeshComponents.Num(),
		PrimarySkeletalMesh ? TEXT("Valid") : TEXT("NULL"));

	// Set up leader pose component for all non-primary skeletal meshes
	if (PrimarySkeletalMesh)
	{
		for (USceneComponent* Component : SpawnedMeshComponents)
		{
			USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(Component);
			if (SkelMesh && SkelMesh != PrimarySkeletalMesh)
			{
				SkelMesh->SetLeaderPoseComponent(PrimarySkeletalMesh);
				//UE_LOG(LogTemp, Log, TEXT("UnitVisualsComponent: Set leader pose component for %s"), *SkelMesh->GetName());

#if WITH_EDITOR
				// Force update in editor to show animation immediately
				SkelMesh->TickAnimation(0.0f, false);
#endif
			}
		}
	}

	// Second pass: Create static meshes (can now attach to skeletal mesh sockets)
	for (const FUnitMeshDescriptor& MeshDesc : Definition->MeshComponents)
	{
		if (MeshDesc.MeshType == EUnitMeshType::Static)
		{
			CreateMeshComponent(MeshDesc, Definition);
		}
	}
}

void UUnitVisualsComponent::CreateMeshComponent(const FUnitMeshDescriptor& Descriptor, UUnitDefinition* Definition)
{
	USceneComponent* NewMeshComponent = nullptr;
	UPrimitiveComponent* PrimitiveComp = nullptr;

	if (Descriptor.MeshType == EUnitMeshType::Skeletal)
	{
		UE_LOG(LogTemp, Log, TEXT("UUnitVisualsComponent: Processing skeletal mesh, bIsPrimaryMesh=%s, IsNull=%s"),
			Descriptor.bIsPrimaryMesh ? TEXT("TRUE") : TEXT("FALSE"),
			Descriptor.SkeletalMesh.IsNull() ? TEXT("TRUE") : TEXT("FALSE"));

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
					UE_LOG(LogTemp, Log, TEXT("UUnitVisualsComponent: Skeletal mesh loaded: %s"), *LoadedMesh->GetName());
					SkelMeshComp->SetSkeletalMesh(LoadedMesh);

					// Enable ticking for animations
					SkelMeshComp->SetComponentTickEnabled(true);
					SkelMeshComp->PrimaryComponentTick.bCanEverTick = true;

					// CRITICAL: Set mobility to Movable so it can move with actor
					SkelMeshComp->SetMobility(EComponentMobility::Movable);

#if WITH_EDITOR
					// Enable animation updates in editor viewport
					SkelMeshComp->SetUpdateAnimationInEditor(true);
#endif

					NewMeshComponent = SkelMeshComp;
					PrimitiveComp = SkelMeshComp;

					if (Descriptor.bIsPrimaryMesh && !PrimarySkeletalMesh)
					{
						PrimarySkeletalMesh = SkelMeshComp;
						UE_LOG(LogTemp, Log, TEXT("UUnitVisualsComponent: Primary skeletal mesh set"));
					}
				}
			}
			//else
			//{
			//	UE_LOG(LogTemp, Error, TEXT("UnitVisualsComponent: Failed to create USkeletalMeshComponent"));
			//}
		}
		//else
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("UnitVisualsComponent: Skeletal mesh descriptor has null mesh reference"));
		//}
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

					// CRITICAL: Set mobility to Movable so it can move with actor
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

	// Attach to parent BEFORE registering (socket or root)
	if (Descriptor.ParentSocket != NAME_None && PrimarySkeletalMesh)
	{
		//UE_LOG(LogTemp, Log, TEXT("UnitVisualsComponent: Attaching mesh to socket '%s' on primary skeletal mesh"),
		//	*Descriptor.ParentSocket.ToString());

		// Use snap rules for socket attachment if no custom transform specified
		FAttachmentTransformRules AttachRules = Descriptor.RelativeTransform.Equals(FTransform::Identity)
			? FAttachmentTransformRules::SnapToTargetIncludingScale
			: FAttachmentTransformRules::KeepRelativeTransform;

		NewMeshComponent->AttachToComponent(PrimarySkeletalMesh, AttachRules, Descriptor.ParentSocket);

		// Verify socket exists
		//if (PrimarySkeletalMesh->DoesSocketExist(Descriptor.ParentSocket))
		//{
		//	UE_LOG(LogTemp, Log, TEXT("UnitVisualsComponent: Socket '%s' exists and mesh attached"),
		//		*Descriptor.ParentSocket.ToString());
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("UnitVisualsComponent: Socket '%s' does NOT exist on skeletal mesh!"),
		//		*Descriptor.ParentSocket.ToString());
		//}

		// Apply custom transform if specified
		if (!Descriptor.RelativeTransform.Equals(FTransform::Identity))
		{
			NewMeshComponent->SetRelativeTransform(Descriptor.RelativeTransform);
		}
	}
	else
	{
		//if (Descriptor.ParentSocket != NAME_None)
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("UnitVisualsComponent: Cannot attach to socket '%s' - PrimarySkeletalMesh is NULL"),
		//		*Descriptor.ParentSocket.ToString());
		//}
		NewMeshComponent->AttachToComponent(VisualsRoot, FAttachmentTransformRules::KeepRelativeTransform);
		NewMeshComponent->SetRelativeTransform(Descriptor.RelativeTransform);
	}

	// Register component AFTER attachment
	NewMeshComponent->RegisterComponent();
	SpawnedMeshComponents.Add(NewMeshComponent);

	UE_LOG(LogTemp, Warning, TEXT("UUnitVisualsComponent: Registered %s, AttachParent=%s, IsRegistered=%s"),
		*NewMeshComponent->GetName(),
		NewMeshComponent->GetAttachParent() ? *NewMeshComponent->GetAttachParent()->GetName() : TEXT("NULL"),
		NewMeshComponent->IsRegistered() ? TEXT("YES") : TEXT("NO"));

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
		WeaponMeshComp->AttachToComponent(PrimarySkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
		WeaponMeshComp->RegisterComponent();

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

void UUnitVisualsComponent::PlayAttackMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage || !PrimarySkeletalMesh)
	{
		return;
	}

	UAnimInstance* AnimInstance = PrimarySkeletalMesh->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(Montage, PlayRate);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UUnitVisualsComponent::HandleMontageCompleted);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}
}

void UUnitVisualsComponent::PlayHitReactionMontage(UAnimMontage* Montage)
{
	if (!Montage || !PrimarySkeletalMesh)
	{
		return;
	}

	UAnimInstance* AnimInstance = PrimarySkeletalMesh->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(Montage);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UUnitVisualsComponent::HandleMontageCompleted);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}
}

void UUnitVisualsComponent::PlayDeathMontage(UAnimMontage* Montage)
{
	if (!Montage || !PrimarySkeletalMesh)
	{
		return;
	}
	UAnimInstance* AnimInstance = PrimarySkeletalMesh->GetAnimInstance();
	if (AnimInstance)
	{
		// Play with 0 blend out - stays on final frame
		AnimInstance->Montage_Play(Montage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
	}
}

void UUnitVisualsComponent::StopAllMontages()
{
	if (PrimarySkeletalMesh)
	{
		UAnimInstance* AnimInstance = PrimarySkeletalMesh->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->StopAllMontages(0.25f);
		}
	}
}

void UUnitVisualsComponent::SetMovementSpeed(float Speed)
{
	if (!PrimarySkeletalMesh)
	{
		return;
	}

	UAnimInstance* AnimInstance = PrimarySkeletalMesh->GetAnimInstance();
	if (AnimInstance)
	{
		FProperty* Property = AnimInstance->GetClass()->FindPropertyByName(FName("MovementSpeed"));
		if (Property && Property->IsA<FFloatProperty>())
		{
			Property->SetValue_InContainer(AnimInstance, &Speed);
		}
	}
}

void UUnitVisualsComponent::SetIsMoving(bool bMoving)
{
	if (!PrimarySkeletalMesh)
	{
		return;
	}

	UAnimInstance* AnimInstance = PrimarySkeletalMesh->GetAnimInstance();
	if (AnimInstance)
	{
		FProperty* Property = AnimInstance->GetClass()->FindPropertyByName(FName("bIsMoving"));
		if (Property && Property->IsA<FBoolProperty>())
		{
			Property->SetValue_InContainer(AnimInstance, &bMoving);
		}
	}
}

void UUnitVisualsComponent::RotateTowardTarget(FRotator TargetRotation, float Speed)
{
	PendingRotation = TargetRotation;
	CurrentRotationSpeed = Speed;
	bIsRotating = true;
}

void UUnitVisualsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsRotating)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			FRotator CurrentRotation = Owner->GetActorRotation();
			FRotator NewRotation = FMath::RInterpTo(CurrentRotation, PendingRotation, DeltaTime, CurrentRotationSpeed);
			Owner->SetActorRotation(NewRotation);

			float RotationDifference = FMath::Abs((PendingRotation - NewRotation).Yaw);
			if (RotationDifference < 1.0f)
			{
				Owner->SetActorRotation(PendingRotation);
				bIsRotating = false;
				OnRotationCompleted.Broadcast();
			}
		}
	}
}

void UUnitVisualsComponent::HandleMontageCompleted(UAnimMontage* Montage, bool bInterrupted)
{
	OnMontageCompleted.Broadcast(Montage);
}

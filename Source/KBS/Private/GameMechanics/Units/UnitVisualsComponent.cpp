#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Tactical/PresentationSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
UUnitVisualsComponent::UUnitVisualsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetMobility(EComponentMobility::Movable);
}

void UUnitVisualsComponent::BeginPlay()
{
	Super::BeginPlay();
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	checkf(OwnerUnit, TEXT("UUnitVisualsComponent must be owned by AUnit"));
	OwnerUnit->OnUnitDied.AddDynamic(this, &UUnitVisualsComponent::OnOwnerDied);
	OwnerUnit->OnUnitDamaged.AddDynamic(this, &UUnitVisualsComponent::OnOwnerDamaged);
	OwnerUnit->OnUnitEffectTriggered.AddDynamic(this, &UUnitVisualsComponent::OnOwnerEffectTriggered);
	OwnerUnit->OnUnitMoved.AddDynamic(this, &UUnitVisualsComponent::OnOwnerMoved);
	OwnerUnit->OnOrientationChanged.AddUObject(this, &UUnitVisualsComponent::OnOwnerOrientationChanged);
}

void UUnitVisualsComponent::OnOwnerDied(AUnit* Unit)
{
	const UUnitDefinition* Def = Unit->GetUnitDefinition();
	if (Def && Def->DeathMontage)
	{
		PlayDeathMontage(Def->DeathMontage);
	}
}

void UUnitVisualsComponent::OnOwnerDamaged(AUnit* Victim, AUnit* Attacker)
{
	if (Victim->IsDead()) return;
	const UUnitDefinition* Def = Victim->GetUnitDefinition();
	if (Def && Def->HitReactionMontage)
	{
		PlayHitReactionMontage(Def->HitReactionMontage);
	}
}
void UUnitVisualsComponent::OnOwnerEffectTriggered(AUnit* OwnerUnit, UBattleEffect* Effect)
{
	ShowBattleEffect(Effect);
}

void UUnitVisualsComponent::OnOwnerOrientationChanged(EUnitOrientation NewOrientation)
{
	if (bIsTranslating || bIsFinalRotating) return;
	GetOwner()->SetActorRotation(OrientationToRotation(NewOrientation));
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

void UUnitVisualsComponent::OnOwnerMoved(AUnit* Unit, const FTacMovementVisualData& MovementData)
{
	if (MovementData.Segments.IsEmpty()) return;

	ActiveMovement = MovementData;
	MovementSegmentIndex = 0;
	bIsTranslating = true;
	bIsFinalRotating = false;

	UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(this);
	FBatchHandle MoveBatch = PresentationSys->BeginBatch(
		FString::Printf(TEXT("Move_%s"), *GetOwner()->GetName())
	);
	CurrentMovementOperation = PresentationSys->RegisterOperation(
		FString::Printf(TEXT("Move_%s"), *GetOwner()->GetName()),
		MoveBatch
	);
	PresentationSys->EndBatch(MoveBatch);

	SetMovementSpeed(Unit->GetMovementSpeed());
	SetIsMoving(true);
}

void UUnitVisualsComponent::CompleteMovementOperation()
{
	if (CurrentMovementOperation.IsValid())
	{
		UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(this);
		PresentationSys->UnregisterOperation(CurrentMovementOperation);
		CurrentMovementOperation = FOperationHandle();
	}
}
void UUnitVisualsComponent::ShowBattleEffect(UBattleEffect* Effect)
{
	const UBattleEffectDataAsset* EffectConfig = Effect->GetConfig();
	if (!EffectConfig || EffectConfig->AppliedVFX.IsNull())
	{
		return;
	}
	UNiagaraSystem* VFX = EffectConfig->AppliedVFX.Get();
	if (!VFX)
	{
		return;
	}
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	SpawnNiagaraEffect(
		VFX,
		OwnerUnit->GetActorLocation() + FVector(0, 0, OwnerUnit->GetSimpleCollisionHalfHeight()),
		EffectConfig->VFXDuration
	);
}
void UUnitVisualsComponent::ClearAllMeshComponents()
{
	//UE_LOG(LogTemp, Warning, TEXT("UUnitVisualsComponent::ClearAllMeshComponents - Clearing %d components"), SpawnedMeshComponents.Num());
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
	//UE_LOG(LogTemp, Warning, TEXT("UUnitVisualsComponent: Owner=%s, VisualsRoot=%s, This IsRegistered=%s, MeshComponents Count=%d"),
	//	GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"),
	//	VisualsRoot ? TEXT("Valid") : TEXT("NULL"),
	//	IsRegistered() ? TEXT("YES") : TEXT("NO"),
	//	Definition->MeshComponents.Num());
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
		SetupAnimationDelegates();
	}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("UUnitVisualsComponent: Cannot init animation - PrimaryMesh: %s, AnimClass: %s"),
	//		PrimarySkeletalMesh ? TEXT("Valid") : TEXT("NULL"),
	//		Definition->AnimationClass ? TEXT("Valid") : TEXT("NULL"));
	//}
	//UE_LOG(LogTemp, Warning, TEXT("UUnitVisualsComponent::InitializeFromDefinition COMPLETE - SpawnedMeshComponents: %d, PrimarySkeletalMesh: %s"),
	//	SpawnedMeshComponents.Num(),
	//	PrimarySkeletalMesh ? TEXT("Valid") : TEXT("NULL"));
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
void UUnitVisualsComponent::CreateMeshComponent(const FUnitMeshDescriptor& Descriptor, UUnitDefinition* Definition)
{
	USceneComponent* NewMeshComponent = nullptr;
	UPrimitiveComponent* PrimitiveComp = nullptr;
	if (Descriptor.MeshType == EUnitMeshType::Skeletal)
	{
		//UE_LOG(LogTemp, Log, TEXT("UUnitVisualsComponent: Processing skeletal mesh, bIsPrimaryMesh=%s, IsNull=%s"),
		//	Descriptor.bIsPrimaryMesh ? TEXT("TRUE") : TEXT("FALSE"),
		//	Descriptor.SkeletalMesh.IsNull() ? TEXT("TRUE") : TEXT("FALSE"));
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
					//UE_LOG(LogTemp, Log, TEXT("UUnitVisualsComponent: Skeletal mesh loaded: %s"), *LoadedMesh->GetName());
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
					//	UE_LOG(LogTemp, Log, TEXT("UUnitVisualsComponent: Primary skeletal mesh set"));
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
	//UE_LOG(LogTemp, Warning, TEXT("UUnitVisualsComponent: Registered %s, AttachParent=%s, IsRegistered=%s"),
	//	*NewMeshComponent->GetName(),
	//	NewMeshComponent->GetAttachParent() ? *NewMeshComponent->GetAttachParent()->GetName() : TEXT("NULL"),
	//	NewMeshComponent->IsRegistered() ? TEXT("YES") : TEXT("NO"));
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
void UUnitVisualsComponent::SetupAnimationDelegates()
{
	UAnimInstance* AnimInstance = PrimarySkeletalMesh->GetAnimInstance();
	checkf(AnimInstance, TEXT("SetupAnimationDelegates: AnimInstance must be valid after InitAnim"));
	AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UUnitVisualsComponent::HandleMontageBlendingOut);
	AnimInstance->OnMontageEnded.AddDynamic(this, &UUnitVisualsComponent::HandleMontageEnded);
}

void UUnitVisualsComponent::PlayAttackMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage || !PrimarySkeletalMesh)
	{
		return;
	}
	UAnimInstance* AnimInstance = PrimarySkeletalMesh->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}
	if (AnimInstance->Montage_Play(Montage, PlayRate) > 0.0f)
	{
		RegisterMontageOperation(Montage);
	}
}

FBatchHandle UUnitVisualsComponent::PlayAttackSequence(AUnit* OwnerUnit, AUnit* Target, UWeapon* Weapon)
{
	checkf(OwnerUnit && Target && Weapon, TEXT("PlayAttackSequence: all params must be valid"));

	UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(this);
	FBatchHandle Batch = PresentationSys->BeginBatch(
		FString::Printf(TEXT("AttackSeq_%s"), *OwnerUnit->GetName())
	);

	const UWeaponDataAsset* WeaponConfig = Weapon->GetConfig();
	UAnimMontage* AttackMontage = WeaponConfig ? WeaponConfig->AttackMontage : nullptr;

	const FVector SourceLoc = OwnerUnit->GetActorLocation();
	const FVector TargetLoc = Target->GetActorLocation();
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(SourceLoc, TargetLoc);
	LookAtRotation.Yaw += MeshYawOffset;

	RegisterRotationOperation(Batch);
	RotateTowardTarget(LookAtRotation, AttackRotationSpeed);

	if (AttackMontage)
	{
		if (UAnimInstance* AnimInstance = PrimarySkeletalMesh ? PrimarySkeletalMesh->GetAnimInstance() : nullptr)
		{
			if (AnimInstance->Montage_Play(AttackMontage) > 0.0f)
			{
				RegisterMontageOperation(AttackMontage, Batch);
			}
		}
	}

	PresentationSys->EndBatch(Batch);
	return Batch;
}

void UUnitVisualsComponent::PlayHitReactionMontage(UAnimMontage* Montage)
{
	if (!Montage || !PrimarySkeletalMesh)
	{
		return;
	}
	UAnimInstance* AnimInstance = PrimarySkeletalMesh->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}
	if (AnimInstance->Montage_Play(Montage) > 0.0f)
	{
		RegisterMontageOperation(Montage);
	}
}
void UUnitVisualsComponent::PlayDeathMontage(UAnimMontage* Montage)
{
	if (!Montage || !PrimarySkeletalMesh)
	{
		return;
	}
	UAnimInstance* AnimInstance = PrimarySkeletalMesh->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}
	if (AnimInstance->Montage_Play(Montage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true) > 0.0f)
	{
		RegisterMontageOperation(Montage);
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

	if (bIsTranslating)
	{
		AActor* Owner = GetOwner();
		const FTacMovementSegment& Seg = ActiveMovement.Segments[MovementSegmentIndex];
		ActiveMovement.CurrentSegmentProgress += DeltaTime;
		const float t = FMath::Clamp(ActiveMovement.CurrentSegmentProgress / Seg.Duration, 0.0f, 1.0f);

		Owner->SetActorLocation(FMath::Lerp(Seg.Start, Seg.End, t));
		Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), Seg.TargetRotation, DeltaTime, 720.0f));

		if (t >= 1.0f)
		{
			ActiveMovement.CurrentSegmentProgress = 0.0f;
			++MovementSegmentIndex;
			if (MovementSegmentIndex >= ActiveMovement.Segments.Num())
			{
				Owner->SetActorLocation(ActiveMovement.Segments.Last().End);
				bIsTranslating = false;
				SetIsMoving(false);
				SetMovementSpeed(0.0f);

				if (ActiveMovement.bApplyDefaultRotationAtEnd || ActiveMovement.bApplyFlankRotationAtEnd)
				{
					bIsFinalRotating = true;
				}
				else
				{
					CompleteMovementOperation();
				}
			}
		}
	}

	if (bIsFinalRotating)
	{
		AActor* Owner = GetOwner();
		const FRotator NewRot = FMath::RInterpTo(Owner->GetActorRotation(), ActiveMovement.TargetRotation, DeltaTime, 360.0f);
		Owner->SetActorRotation(NewRot);
		if (FMath::Abs(FRotator::NormalizeAxis(ActiveMovement.TargetRotation.Yaw - NewRot.Yaw)) < 1.0f)
		{
			Owner->SetActorRotation(ActiveMovement.TargetRotation);
			bIsFinalRotating = false;
			CompleteMovementOperation();
		}
	}

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
				CompleteRotationOperation();
				OnRotationCompleted.Broadcast();
				OnRotationCompletedNative.Broadcast();
			}
		}
	}
}
void UUnitVisualsComponent::HandleMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	// Complete subsystem op early on blend-out so downstream systems unblock without waiting for full blend
	FOperationHandle* Handle = ActiveMontageOperations.Find(Montage);
	if (Handle && Handle->IsValid())
	{
		UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(this);
		PresentationSys->UnregisterOperation(*Handle);
		ActiveMontageOperations.Remove(Montage);
	}
}

void UUnitVisualsComponent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// If blend-out didn't fire (e.g. blend time = 0 or immediate stop), complete here
	FOperationHandle* Handle = ActiveMontageOperations.Find(Montage);
	if (Handle && Handle->IsValid())
	{
		UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(this);
		PresentationSys->UnregisterOperation(*Handle);
		ActiveMontageOperations.Remove(Montage);
	}
	OnMontageCompleted.Broadcast(Montage);
	OnMontageCompletedNative.Broadcast(Montage);
}
UNiagaraComponent* UUnitVisualsComponent::SpawnNiagaraEffect(UNiagaraSystem* System, FVector WorldLocation, float Duration)
{
	if (!System)
	{
		return nullptr;
	}
	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		System,
		WorldLocation,
		FRotator::ZeroRotator,
		FVector::OneVector,
		true,
		true,
		ENCPoolMethod::None,
		true
	);

	if (NiagaraComponent && Duration > 0.0f)
	{
		UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(this);
		if (PresentationSys)
		{
			// Create RAII scoped operation for VFX
			TSharedPtr<UPresentationSubsystem::FScopedOperation> VFXOp =
				MakeShared<UPresentationSubsystem::FScopedOperation>(
					PresentationSys,
					FString::Printf(TEXT("VFX: %s"), *System->GetName())
				);

			FVFXTrackingData TrackingData;
			TrackingData.ScopedOperation = VFXOp;

			FTimerHandle TimerHandle;
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUObject(this, &UUnitVisualsComponent::OnVFXCompleted, NiagaraComponent);
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);
			TrackingData.TimerHandle = TimerHandle;

			ActiveVFXOperations.Add(NiagaraComponent, TrackingData);
		}
	}

	return NiagaraComponent;
}

void UUnitVisualsComponent::OnVFXCompleted(UNiagaraComponent* Component)
{
	if (!Component)
	{
		return;
	}

	FVFXTrackingData* TrackingData = ActiveVFXOperations.Find(Component);
	if (TrackingData)
	{
		// ScopedOperation will auto-complete when removed from map
		ActiveVFXOperations.Remove(Component);
	}
}

void UUnitVisualsComponent::RegisterRotationOperation(FBatchHandle BatchHandle)
{
	UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(this);
	CurrentRotationOperation = PresentationSys->RegisterOperation(
		FString::Printf(TEXT("Rotation_%s"), *GetOwner()->GetName()),
		BatchHandle
	);
}

void UUnitVisualsComponent::RegisterMontageOperation(UAnimMontage* Montage, FBatchHandle BatchHandle)
{
	// Montages with bEnableAutoBlendOut=false hold the last frame forever and never fire end/blendout events.
	// Treat them as fire-and-forget — don't register with the subsystem or we'd stall indefinitely.
	if (!Montage->bEnableAutoBlendOut)
	{
		return;
	}
	UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(this);
	FOperationHandle Handle = PresentationSys->RegisterOperation(
		FString::Printf(TEXT("Montage_%s_%s"), *GetOwner()->GetName(), *Montage->GetName()),
		BatchHandle
	);
	ActiveMontageOperations.Add(Montage, Handle);
}

void UUnitVisualsComponent::CompleteRotationOperation()
{
	if (CurrentRotationOperation.IsValid())
	{
		UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(this);
		PresentationSys->UnregisterOperation(CurrentRotationOperation);
		CurrentRotationOperation = FOperationHandle();
	}
}

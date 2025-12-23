#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/AbilityFactory.h"
#include "GameplayTypes/CombatTypes.h"
AUnit::AUnit()
{
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	SceneRoot->SetMobility(EComponentMobility::Movable);
	VisualsComponent = CreateDefaultSubobject<UUnitVisualsComponent>(TEXT("VisualsComponent"));
	VisualsComponent->SetupAttachment(RootComponent);
	EffectManager = CreateDefaultSubobject<UBattleEffectComponent>(TEXT("EffectManager"));
	AbilityInventory = CreateDefaultSubobject<UAbilityInventoryComponent>(TEXT("AbilityInventory"));
}
void AUnit::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Error, TEXT("===== AUnit::BeginPlay START - Actor: %s ====="), *GetName());
	if (!UnitDefinition)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unit has no UnitDefinition assigned!"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("AUnit::BeginPlay - UnitDefinition: %s"), *UnitDefinition->GetName());
	if (VisualsComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AUnit::BeginPlay - Calling InitializeFromDefinition"));
		VisualsComponent->InitializeFromDefinition(UnitDefinition);
		UE_LOG(LogTemp, Error, TEXT("AUnit::BeginPlay - AFTER Init: SpawnedMeshComponents=%d, PrimaryMesh=%s"),
			VisualsComponent->GetAllMeshComponents().Num(),
			VisualsComponent->GetPrimarySkeletalMesh() ? TEXT("Valid") : TEXT("NULL"));
	}
	BaseStats = UnitDefinition->BaseStatsTemplate;
	Progression = UnitDefinition->BaseProgressionTemplate;
	CurrentHealth = BaseStats.MaxHealth;
	for (int32 i = 1; i < InitialLevel; ++i)
	{
		LevelUp();
	}
	for (UWeaponDataAsset* WeaponData : UnitDefinition->DefaultWeapons)
	{
		if (WeaponData)
		{
			UWeapon* NewWeapon = NewObject<UWeapon>(this, UWeapon::StaticClass());
			if (NewWeapon)
			{
				NewWeapon->RegisterComponent();
				NewWeapon->Initialize(VisualsComponent, WeaponData);
				Weapons.Add(NewWeapon);
			}
		}
	}
	RecalculateModifiedStats();
	if (AbilityInventory && UnitDefinition)
	{
		for (UUnitAbilityDefinition* AbilityDef : UnitDefinition->DefaultAbilities)
		{
			if (!AbilityDef)
			{
				continue;
			}
			UUnitAbilityInstance* NewAbility = UAbilityFactory::CreateAbilityFromDefinition(AbilityDef, this);
			if (!NewAbility)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to create ability from definition '%s'"), *AbilityDef->GetName());
				continue;
			}
			if (NewAbility->IsPassive())
			{
				AbilityInventory->AddPassiveAbility(NewAbility);
			}
			else
			{
				AbilityInventory->AddActiveAbility(NewAbility);
			}
		}
		AbilityInventory->EquipDefaultAbility();
		AbilityInventory->RegisterPassives();
	}
}
void AUnit::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
#if WITH_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("AUnit::OnConstruction - Actor: %s, WorldType: %d"),
		*GetName(),
		GetWorld() ? (int32)GetWorld()->WorldType : -1);
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		UE_LOG(LogTemp, Warning, TEXT("AUnit::OnConstruction - Editor world, initializing visuals"));
		if (UnitDefinition && VisualsComponent)
		{
			VisualsComponent->ClearAllMeshComponents();
			VisualsComponent->InitializeFromDefinition(UnitDefinition);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AUnit::OnConstruction - NOT Editor world (WorldType=%d), skipping visual init"),
			GetWorld() ? (int32)GetWorld()->WorldType : -1);
	}
#endif
}
void AUnit::RecalculateModifiedStats()
{
	ModifiedStats = BaseStats;
	for (auto& ArmorPair : ModifiedStats.Defense.Armour)
	{
		ArmorPair.Value = FMath::Clamp(ArmorPair.Value, 0.0f, 0.9f);
	}
}
void AUnit::RecalculateAllWeaponStats()
{
	for (TObjectPtr<UWeapon> Weapon : Weapons)
	{
		if (Weapon)
		{
			Weapon->RecalculateModifiedStats();
		}
	}
}
void AUnit::Restore()
{
	ModifiedStats = BaseStats;
	for (TObjectPtr<UWeapon> Weapon : Weapons)
	{
		if (Weapon)
		{
			Weapon->Restore();
		}
	}
}
void AUnit::LevelUp()
{
	const float HealthPercentage = BaseStats.MaxHealth > 0.0f ? CurrentHealth / BaseStats.MaxHealth : 1.0f;
	BaseStats.MaxHealth *= 1.1f;
	BaseStats.Accuracy = FMath::Min(BaseStats.Accuracy + 0.01f, 1.0f);
	CurrentHealth = BaseStats.MaxHealth * HealthPercentage;
	Progression.LevelOnCurrentTier++;
	RecalculateModifiedStats();
}
void AUnit::SetUnitDefinition(UUnitDefinition* InDefinition)
{
	UnitDefinition = InDefinition;
}
FUnitDisplayData AUnit::GetDisplayData() const
{
	const FString Name = UnitDefinition ? UnitDefinition->UnitName : TEXT("Unknown");
	UTexture2D* Portrait = UnitDefinition ? UnitDefinition->Portrait : nullptr;
	const TArray<TObjectPtr<UBattleEffect>> Effects = EffectManager ? EffectManager->GetActiveEffects() : TArray<TObjectPtr<UBattleEffect>>();
	return BuildUnitDisplayData(Name, CurrentHealth, ModifiedStats, Progression, Portrait, Effects, Weapons, TeamSide);
}
void AUnit::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	UE_LOG(LogTemp, Warning, TEXT("[EVENT] Unit '%s' clicked with button '%s'"), *GetName(), *ButtonPressed.ToString());
	OnUnitClicked.Broadcast(this, ButtonPressed);
}
const float AUnit::GetMovementSpeed() const
	{ return UnitDefinition->MovementSpeed; }
void AUnit::TakeHit(const FDamageResult& DamageResult)
{
	CurrentHealth -= DamageResult.Damage;
	CurrentHealth = FMath::Max(0.0f, CurrentHealth);
	if (DamageResult.DamageBlocked > 0 && ModifiedStats.Defense.Wards.Contains(DamageResult.DamageSource))
	{
		ModifiedStats.Defense.Wards.Remove(DamageResult.DamageSource);
	}
	if (CurrentHealth <= 0.0f)
	{
		if (EffectManager)
		{
			EffectManager->BroadcastDied();
		}
		if (VisualsComponent && UnitDefinition && UnitDefinition->DeathMontage)
		{
			VisualsComponent->PlayDeathMontage(UnitDefinition->DeathMontage);
		}
		UE_LOG(LogTemp, Warning, TEXT("[EVENT] Broadcasting OnUnitDied for unit '%s'"), *GetName());
		OnUnitDied.Broadcast(this);
	}
	else
	{
		if (VisualsComponent && UnitDefinition && UnitDefinition->HitReactionMontage)
		{
			VisualsComponent->PlayHitReactionMontage(UnitDefinition->HitReactionMontage);
		}
	}
}
void AUnit::ApplyEffect(UBattleEffect* Effect)
{
	if (EffectManager && Effect)
	{
		EffectManager->AddEffect(Effect);
	}
}
void AUnit::SetDefending(bool bDefending)
{
	ModifiedStats.Defense.bIsDefending = bDefending;
}
void AUnit::OnUnitTurnStart()
{
	UE_LOG(LogTemp, Log, TEXT("%s: Turn started"), *GetName());
	ModifiedStats.Defense.bIsDefending = false;
	if (EffectManager)
	{
		EffectManager->BroadcastTurnStart();
	}
	for (TObjectPtr<UWeapon> Weapon : Weapons)
	{
		if (Weapon)
		{
		}
	}
}
void AUnit::OnUnitTurnEnd()
{
	UE_LOG(LogTemp, Log, TEXT("%s: Turn ended"), *GetName());
	if (EffectManager)
	{
		EffectManager->BroadcastTurnEnd();
	}
}

#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Abilities/UnitAutoAttackAbility.h"
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

	if (!UnitDefinition)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unit has no UnitDefinition assigned!"));
		return;
	}

	// Initialize visual components from definition
	if (VisualsComponent)
	{
		VisualsComponent->InitializeFromDefinition(UnitDefinition);
	}

	// Initialize base stats and progression from template
	BaseStats = UnitDefinition->BaseStatsTemplate;
	Progression = UnitDefinition->BaseProgressionTemplate;
	CurrentHealth = BaseStats.MaxHealth;

	// Apply initial leveling (stub for now)
	for (int32 i = 1; i < InitialLevel; ++i)
	{
		// TODO: Implement proper levelup logic
		LevelUp();
	}

	// Create weapon components from definition
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

	// Create and equip default StandardAttackAction
	if (AbilityInventory)
	{
		// Create default ability definition
		UUnitAbilityDefinition* DefaultAbilityDef = NewObject<UUnitAbilityDefinition>(this);
		if (DefaultAbilityDef)
		{
			DefaultAbilityDef->AbilityName = TEXT("Standard Attack");
			DefaultAbilityDef->MaxCharges = -1; // Unlimited charges
			DefaultAbilityDef->Targeting = ETargetReach::None;
			DefaultAbilityDef->bIsPassive = false;

			// Create ability instance
			UUnitAutoAttackAbility* StandardAttack = NewObject<UUnitAutoAttackAbility>(this);
			if (StandardAttack)
			{
				StandardAttack->InitializeFromDefinition(DefaultAbilityDef, this);
				AbilityInventory->EquipAbility(StandardAttack);
			}
		}

		// Register passive abilities with the subsystem
		AbilityInventory->RegisterPassives();
	}
}

void AUnit::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		if (UnitDefinition && VisualsComponent)
		{
			VisualsComponent->ClearAllMeshComponents();
			VisualsComponent->InitializeFromDefinition(UnitDefinition);
		}
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
	FUnitDisplayData DisplayData;
	DisplayData.UnitName = UnitDefinition ? UnitDefinition->UnitName : TEXT("Unknown");
	DisplayData.CurrentHealth = CurrentHealth;
	DisplayData.MaxHealth = ModifiedStats.MaxHealth;
	DisplayData.Initiative = ModifiedStats.Initiative;
	DisplayData.Accuracy = ModifiedStats.Accuracy;
	DisplayData.Level = Progression.LevelOnCurrentTier;
	DisplayData.Experience = Progression.TotalExperience;
	DisplayData.ExperienceToNextLevel = Progression.ExperienceToNextLevel;
	DisplayData.Defense = ModifiedStats.Defense;
	DisplayData.PortraitTexture = UnitDefinition ? UnitDefinition->Portrait : nullptr;
	
	UE_LOG(LogTemp, Warning, TEXT("Unit data returned: HP [%f] INI [%d]"), DisplayData.CurrentHealth, DisplayData.Initiative);
	// Get active effects from EffectManager
	// TODO

	return DisplayData;
}

void AUnit::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	UE_LOG(LogTemp, Warning, TEXT("Unit clicked"));
	OnUnitClicked.Broadcast(this);
}

void AUnit::TakeHit(const FDamageResult& DamageResult)
{
	CurrentHealth -= DamageResult.Damage;
	CurrentHealth = FMath::Max(0.0f, CurrentHealth);

	// Remove ward if it was spent during damage calculation
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
	}
}

void AUnit::ApplyEffect(UBattleEffect* Effect)
{
	if (EffectManager && Effect)
	{
		EffectManager->AddEffect(Effect);
	}
}

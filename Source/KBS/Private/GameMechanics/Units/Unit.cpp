#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Units/EffectManagerComponent.h"

AUnit::AUnit()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	EffectManager = CreateDefaultSubobject<UEffectManagerComponent>(TEXT("EffectManager"));
}

void AUnit::BeginPlay()
{
	Super::BeginPlay();

	if (!UnitDefinition)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unit has no UnitDefinition assigned!"));
		return;
	}

	// Initialize mesh and animation
	if (UnitDefinition->Mesh)
	{
		MeshComponent->SetSkeletalMesh(UnitDefinition->Mesh);
	}

	if (UnitDefinition->AnimationClass)
	{
		MeshComponent->SetAnimInstanceClass(UnitDefinition->AnimationClass);
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
				Weapons.Add(NewWeapon);
				// Note: Weapon initialization should be called after spawn
			}
		}
	}

	RecalculateModifiedStats();
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

void AUnit::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	UE_LOG(LogTemp, Warning, TEXT("Unit clicked"));
	OnUnitClicked.Broadcast(this);
}

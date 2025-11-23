#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Weapons/Weapon.h"

AUnit::AUnit()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Fab/Hand_Painted_Paladin_Knight_-_Rigged___Game_Ready/paintpoly_humanoid_paladin_knight.paintpoly_humanoid_paladin_knight"));
	if (MeshAsset.Succeeded())
	{
		MeshComponent->SetSkeletalMesh(MeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> Material(TEXT("/Game/Fab/Hand_Painted_Paladin_Knight_-_Rigged___Game_Ready/m_paladin_knight.m_paladin_knight"));
	if (Material.Succeeded())
	{
		MeshComponent->SetMaterial(0, Material.Object);
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> AnimSequence(TEXT("/Game/Fab/Hand_Painted_Paladin_Knight_-_Rigged___Game_Ready/paintpoly_humanoid_paladin_knight_Anim.paintpoly_humanoid_paladin_knight_Anim"));
	if (AnimSequence.Succeeded())
	{
		MeshComponent->SetAnimation(AnimSequence.Object);
		MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MeshComponent->Play(false);
	}
}

void AUnit::SetGridPosition(int32 Row, int32 Col, EBattleLayer Layer)
{
	GridRow = Row;
	GridCol = Col;
	GridLayer = Layer;
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

	UE_LOG(LogTemp, Warning, TEXT("Unit clicked at [%d,%d]"), GridRow, GridCol);
	OnUnitClicked.Broadcast(this);
}

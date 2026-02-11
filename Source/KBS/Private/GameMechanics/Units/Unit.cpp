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
#include "GameMechanics/Tactical/PresentationSubsystem.h"
#include "GameplayTypes/CombatTypes.h"
#include "GameplayTypes/AbilityTypes.h"
AUnit::AUnit()
{
	UnitID = FGuid::NewGuid();

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

	// Verify critical components exist
	if (!EffectManager)
	{
		EffectManager = NewObject<UBattleEffectComponent>(this, TEXT("EffectManager"));
		if (EffectManager)
		{
			EffectManager->RegisterComponent();
		}
	}

	if (!UnitDefinition)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unit has no UnitDefinition assigned!"));
		return;
	}
	if (VisualsComponent)
	{
		VisualsComponent->InitializeFromDefinition(UnitDefinition);
	}
	BaseStats.InitFromBase(UnitDefinition->BaseStatsTemplate);
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
	if (AbilityInventory && UnitDefinition)
	{
		// Initialize default slot abilities
		if (UnitDefinition->DefaultAttackAbility)
		{
			UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(UnitDefinition->DefaultAttackAbility, this);
			if (Ability)
			{
				AbilityInventory->SetDefaultAbility(EDefaultAbilitySlot::Attack, Ability);
				AbilityInventory->AddActiveAbility(Ability);
			}
		}

		if (UnitDefinition->DefaultMoveAbility)
		{
			UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(UnitDefinition->DefaultMoveAbility, this);
			if (Ability)
			{
				AbilityInventory->SetDefaultAbility(EDefaultAbilitySlot::Move, Ability);
				AbilityInventory->AddActiveAbility(Ability);
			}
		}

		if (UnitDefinition->DefaultWaitAbility)
		{
			UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(UnitDefinition->DefaultWaitAbility, this);
			if (Ability)
			{
				AbilityInventory->SetDefaultAbility(EDefaultAbilitySlot::Wait, Ability);
				AbilityInventory->AddActiveAbility(Ability);
			}
		}

		if (UnitDefinition->DefaultDefendAbility)
		{
			UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(UnitDefinition->DefaultDefendAbility, this);
			if (Ability)
			{
				AbilityInventory->SetDefaultAbility(EDefaultAbilitySlot::Defend, Ability);
				AbilityInventory->AddActiveAbility(Ability);
			}
		}

		if (UnitDefinition->DefaultFleeAbility)
		{
			UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(UnitDefinition->DefaultFleeAbility, this);
			if (Ability)
			{
				AbilityInventory->SetDefaultAbility(EDefaultAbilitySlot::Flee, Ability);
				AbilityInventory->AddActiveAbility(Ability);
			}
		}

		// Initialize additional abilities
		for (UUnitAbilityDefinition* AbilityDef : UnitDefinition->AdditionalAbilities)
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
void AUnit::SetUnitDefinition(UUnitDefinition* InDefinition)
{
	UnitDefinition = InDefinition;
	if (UnitDefinition)
	{
		BaseStats.InitFromBase(UnitDefinition->BaseStatsTemplate);

		// Recreate weapons from new definition
		Weapons.Empty();
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
	}
}
FUnitDisplayData AUnit::GetDisplayData() const
{
	const FString Name = UnitDefinition ? UnitDefinition->UnitName : TEXT("Unknown");
	UTexture2D* Portrait = UnitDefinition ? UnitDefinition->Portrait : nullptr;
	const TArray<TObjectPtr<UBattleEffect>> Effects = EffectManager ? EffectManager->GetActiveEffects() : TArray<TObjectPtr<UBattleEffect>>();
	return BuildUnitDisplayData(Name, BaseStats.Health.GetCurrent(), BaseStats, Portrait, Effects, Weapons, TeamSide);
}
void AUnit::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	UE_LOG(LogTemp, Warning, TEXT("[EVENT] Unit '%s' clicked with button '%s'"), *GetName(), *ButtonPressed.ToString());
	OnUnitClicked.Broadcast(this, ButtonPressed);
}
float AUnit::GetMovementSpeed() const
	{ return UnitDefinition->MovementSpeed; }
void AUnit::TakeHit(const FDamageResult& DamageResult)
{
	BaseStats.Health.TakeDamage(DamageResult.Damage);
	if (DamageResult.DamageBlocked > 0)
	{
		BaseStats.Defense.Wards.UseWard(DamageResult.DamageSource);
	}
	if (BaseStats.Health.IsDead())
	{
		if (EffectManager)
		{
			EffectManager->BroadcastDied();
			// Clear all effects immediately on death to prevent dead units from ticking effects
			EffectManager->ClearAllEffects();
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
		// Only play hit reaction for actual damage (not buffs/heals)
		if (DamageResult.Damage > 0.0f && VisualsComponent && UnitDefinition && UnitDefinition->HitReactionMontage)
		{
			VisualsComponent->RegisterMontageOperation();
			VisualsComponent->PlayHitReactionMontage(UnitDefinition->HitReactionMontage);
		}
	}
}
bool AUnit::ApplyEffect(UBattleEffect* Effect)
{
	UE_LOG(LogTemp, Log, TEXT("AUnit::ApplyEffect - Called on %s with effect %s, EffectManager = %s"),
		*GetName(),
		Effect ? *Effect->GetClass()->GetName() : TEXT("NULL"),
		EffectManager ? TEXT("Valid") : TEXT("NULL"));
	if (EffectManager && Effect)
	{
		return EffectManager->AddEffect(Effect);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AUnit::ApplyEffect - Cannot apply effect (EffectManager or Effect is NULL)"));
		return false;
	}
}
void AUnit::SetDefending(bool bDefending)
{
	BaseStats.Defense.bIsDefending = bDefending;
}
void AUnit::HandleTurnStart()
{
	if (BaseStats.Health.IsDead()) return;
	UE_LOG(LogTemp, Log, TEXT("%s: Turn started"), *GetName());
	BaseStats.Defense.bIsDefending = false;
	if (AbilityInventory)
	{
		AbilityInventory->SelectAttackAbility();
	}
	if (EffectManager)
	{
		EffectManager->BroadcastTurnStart();
	}
	OnUnitTurnStart.Broadcast(this);
}
void AUnit::HandleTurnEnd()
{
	if (BaseStats.Health.IsDead()) return;
	UE_LOG(LogTemp, Log, TEXT("%s: Turn ended"), *GetName());
	if (EffectManager)
	{
		EffectManager->BroadcastTurnEnd();
	}
	OnUnitTurnEnd.Broadcast(this);
}
void AUnit::HandleAttacked(AUnit* Attacker)
{
	if (BaseStats.Health.IsDead()) return;
	if (Attacker == this) return; // cycle guard
	if (EffectManager)
	{
		EffectManager->BroadcastAttacked(Attacker);
	}
	OnUnitAttacked.Broadcast(this, Attacker);
}
void AUnit::HandleAttacks(AUnit* Target)
{
	if (BaseStats.Health.IsDead()) return;
	if (Target == this) return; // cycle guard
	if (EffectManager)
	{
		EffectManager->BroadcastAttacks(Target);
	}
	OnUnitAttacks.Broadcast(this, Target);
}
void AUnit::HandleMoved()
{
	if (BaseStats.Health.IsDead()) return;
	if (EffectManager)
	{
		EffectManager->BroadcastMoved();
	}
	OnUnitMoved.Broadcast(this);
}

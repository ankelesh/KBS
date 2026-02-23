#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectComponent.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameplayTypes/CombatTypes.h"

FString AUnit::GetLogName() const
{
	const UUnitDefinition* Def = GetUnitDefinition();
	const FString& Name = Def ? Def->UnitName : TEXT("?");
	return FString::Printf(TEXT("%s [%s]"), *Name, *UnitID.ToString().Left(8));
}

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

	if (!UnitDefinition)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Unit has no UnitDefinition assigned!"));
		return;
	}
	if (VisualsComponent)
	{
		VisualsComponent->InitializeFromDefinition(UnitDefinition);
	}
	BaseStats.InitFromBase(UnitDefinition->BaseStatsTemplate);
	InitializeWeapons(UnitDefinition);
	AbilityInventory->InitializeFromDefinition(UnitDefinition, this);
}

void AUnit::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
#if WITH_EDITOR
	//UE_LOG(LogTemp, Warning, TEXT("AUnit::OnConstruction - Actor: %s, WorldType: %d"),
	//	*GetName(),
	//		GetWorld() ? (int32)GetWorld()->WorldType : -1);
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		//UE_LOG(LogTemp, Warning, TEXT("AUnit::OnConstruction - Editor world, initializing visuals"));
		if (UnitDefinition && VisualsComponent)
		{
			VisualsComponent->ClearAllMeshComponents();
			VisualsComponent->InitializeFromDefinition(UnitDefinition);
		}
	}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("AUnit::OnConstruction - NOT Editor world (WorldType=%d), skipping visual init"),
	//		GetWorld() ? (int32)GetWorld()->WorldType : -1);
	//}
#endif
}

void AUnit::SetUnitDefinition(UUnitDefinition* InDefinition)
{
	UnitDefinition = InDefinition;
	if (UnitDefinition)
	{
		BaseStats.InitFromBase(UnitDefinition->BaseStatsTemplate);

		Weapons.Empty();
		InitializeWeapons(UnitDefinition);
	}
}

void AUnit::InitializeWeapons(const UUnitDefinition* Definition)
{
	for (const FUnitWeaponEntry& Entry : Definition->DefaultWeapons)
	{
		if (!Entry.Weapon) continue;
		TObjectPtr<UWeapon> NewWeapon = NewObject<UWeapon>();
		NewWeapon->Initialize(this, Entry.Weapon);
		if (Entry.BaseDamageOverride != NoWeaponDamageOverride)
		{
			NewWeapon->GetMutableStats().BaseDamage.SetBase(Entry.BaseDamageOverride);
		}
		Weapons.Add(NewWeapon);
	}
}

FUnitDisplayData AUnit::GetDisplayData() const
{
	const FString Name = UnitDefinition ? UnitDefinition->UnitName : TEXT("Unknown");
	UTexture2D* Portrait = UnitDefinition ? UnitDefinition->Portrait : nullptr;
	const TArray<TObjectPtr<UBattleEffect>> Effects = EffectManager
		                                                  ? EffectManager->GetActiveEffects()
		                                                  : TArray<TObjectPtr<UBattleEffect>>();
	return BuildUnitDisplayData(Name, BaseStats.Health.GetCurrent(), BaseStats, Portrait, Effects, Weapons,
	                            GridMetadata.Team);
}

void AUnit::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	//UE_LOG(LogTemp, Warning, TEXT("[EVENT] Unit '%s' clicked with button '%s'"), *GetName(), *ButtonPressed.ToString());
	OnUnitClicked.Broadcast(this, ButtonPressed);
}

float AUnit::GetMovementSpeed() const
{
	return UnitDefinition->MovementSpeed;
}

void AUnit::HandleHit(const FDamageResult& Result, AUnit* Attacker, bool Emits)
{
	ChangeUnitHP(-Result.Damage);
	if (Result.WardSpent != EDamageSource::None)
		ConsumeWard(Result.WardSpent);
	if (Emits && Result.Damage > 0 && Attacker)
		OnUnitDamaged.Broadcast(this, Attacker);
}

void AUnit::HandleDeath(bool Emits)
{
	BaseStats.Status.SetDead();
	if (Emits) OnUnitDied.Broadcast(this);
}

void AUnit::ChangeUnitHP(int32 Delta, bool Emits)
{
	BaseStats.Health.ApplyDelta(Delta);
	if (Emits) OnHealthChanged.Broadcast(this, BaseStats.Health.GetCurrent());
	if (BaseStats.Health.IsDead()) HandleDeath();
}

void AUnit::ConsumeWard(EDamageSource Source, bool Emits)
{
	BaseStats.Defense.Wards.UseWard(Source);
	if (Emits) OnUnitStatsModified.Broadcast(this, BaseStats);
}

bool AUnit::ApplyEffect(UBattleEffect* Effect, bool Emits)
{
	checkf(Effect, TEXT("ApplyEffect called with null Effect on %s"), *GetLogName());
	checkf(EffectManager, TEXT("EffectManager is null on %s - component may have been GC'd"), *GetLogName());
	if (IsDead()) return false;
	const bool bApplied = EffectManager->AddEffect(Effect);
	if (bApplied && Emits) OnUnitEffectApplied.Broadcast(this, Effect);
	return bApplied;
}

void AUnit::NotifyEffectTriggered(UBattleEffect* Effect)
{
	if (IsDead()) return;
	OnUnitEffectTriggered.Broadcast(this, Effect);
}

void AUnit::HandleTurnStart(bool Emits)
{
	if (IsDead()) return;
	BaseStats.Status.ClearStatus(EUnitStatus::Defending);
	BaseStats.Status.ClearStatus(EUnitStatus::Focused);
	if (Emits) OnUnitTurnStart.Broadcast(this);
}

void AUnit::HandleTurnEnd(bool Emits)
{
	if (IsDead()) return;
	if (Emits) OnUnitTurnEnd.Broadcast(this);
}

void AUnit::HandleAttacked(AUnit* Attacker, bool Emits)
{
	if (IsDead()) return;
	if (Attacker == this) return; // cycle guard
	if (Emits) OnUnitAttacked.Broadcast(this, Attacker);
}

void AUnit::HandleAttacks(AUnit* Target, bool Emits)
{
	if (IsDead()) return;
	if (Target == this) return; // cycle guard
	if (Emits) OnUnitAttacks.Broadcast(this, Target);
}

void AUnit::HandleMoved(const FTacMovementVisualData& MovementData)
{
	if (IsDead()) return;
	OnUnitMoved.Broadcast(this, MovementData);
}

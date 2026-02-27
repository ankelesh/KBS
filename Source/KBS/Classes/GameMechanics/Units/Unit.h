#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameMechanics/Units/Stats/UnitStats.h"
#include "UnitDisplayData.h"
#include "UnitGridMetadata.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameplayTypes/CombatTypes.h"
#include "GameplayTypes/TacMovementTypes.h"
#include "Unit.generated.h"

class UUnitAbilityInstance;
class UUnitDefinition;
class UBattleEffectComponent;
class UBattleEffect;
class UAbilityInventoryComponent;
class UUnitVisualsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitClicked, AUnit*, ClickedUnit, FKey, ButtonPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitAttacked, AUnit*, Victim, AUnit*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitDamaged, AUnit*, Victim, AUnit*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitAttackStarted, AUnit*, Attacker, AUnit*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitDied, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitTurnStarted, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitTurnEnded, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitMoved, AUnit*, Unit, const FTacMovementVisualData&, MovementData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitHealthChanged, AUnit*, Unit, int32, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitEffectApplied, AUnit*, Unit, UBattleEffect*, AppliedEffect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitEffectTriggered, AUnit*, Owner, UBattleEffect*, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitAbilityUsed, AUnit*, Unit, UUnitAbilityInstance*, Ability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitStatusChanged, AUnit*, Unit, EUnitStatus, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitStatsModified, AUnit*, Unit, FUnitCoreStats&, Stats);

/**
 * Unit-level mirrors of UTacCombatSubsystem phase broadcasts.
 *
 * Subsystem emits one global signal per phase; these are re-broadcast per unit
 * (attacker and target separately) so abilities only need to bind to their owner
 * without filtering.
 *
 * Existing coarse signals (OnUnitAttacked / OnUnitAttacks) are NOT replaced —
 * they carry no context structs and remain for lightweight observers (UI, visuals).
 *
 * Phase order per hit:
 *   1. PreAttack  — cancellation window, before accuracy roll
 *   2. Calculation — accuracy roll + damage calc; modifier HERE affects the roll
 *   3. DamageApply — damage is written to target; modify FDamageResult to absorb/amplify
 *   4. EffectApplication — battle effects applied; cancel or add effects here
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUnitCombatCalculation, FAttackContext&, FHitInstance&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnUnitDamageApply, FAttackContext&, FHitInstance&, FDamageResult&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUnitOrientationChanged, EUnitOrientation);

UCLASS(meta=(ScriptName="TacUnit"))
class KBS_API AUnit : public APawn
{
	GENERATED_BODY()
	friend class UGridDataManager;
public:
	// --- Lifecycle ---
	AUnit();
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	void SetUnitDefinition(UUnitDefinition* InDefinition);

	// --- Outgoing Events ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitClicked OnUnitClicked;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitAttacked OnUnitAttacked;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitDamaged OnUnitDamaged;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitDied OnUnitDied;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitTurnStarted OnUnitTurnStart;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitTurnEnded OnUnitTurnEnd;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitAttackStarted OnUnitAttacks;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitMoved OnUnitMoved;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitHealthChanged OnHealthChanged;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitEffectApplied OnUnitEffectApplied;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitAbilityUsed OnUnitAbilityUsed;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitStatusChanged OnUnitStatusChanged;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitStatsModified OnUnitStatsModified;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitEffectTriggered OnUnitEffectTriggered;

	// Subscribe only — broadcast exclusively via NotifyOrientationChanged() (DataManager-owned).
	FOnUnitOrientationChanged OnOrientationChanged;

	// --- Combat Phase Events (unit-scoped mirrors of UTacCombatSubsystem signals) ---
	// Each signal fires only when THIS unit is the attacker or target of the relevant hit.
	// Bind here instead of the subsystem to avoid per-hit owner checks.

	/** Phase 1 — attacker side: fired before accuracy roll; context-level (whole attack). */
	FOnUnitCombatCalculation OnStartingAttack;
	/** Phase 1 — target side: fired before accuracy roll; per-hit. */
	FOnUnitCombatCalculation OnBeingAttackedPrePhase;

	/** Phase 2 — attacker side: fired during accuracy/damage calculation; modify attacker stats here. */
	FOnUnitCombatCalculation OnAttackingInCalculation;
	/** Phase 2 — target side: fired during accuracy/damage calculation; modify attacker stats here. */
	FOnUnitCombatCalculation OnBeingTargetedInCalculation;

	/** Phase 3 — attacker side: fired when damage result is being applied. */
	FOnUnitDamageApply OnAttackingDamageApply;
	/** Phase 3 — target side: fired when damage result is being applied; modify FDamageResult to absorb. */
	FOnUnitDamageApply OnBeingHitInDamageApply;

	/** Phase 4 — attacker side: fired during effect application. */
	FOnUnitCombatCalculation OnAttackingEffectApplication;
	/** Phase 4 — target side: fired during effect application; cancel or modify effects here. */
	FOnUnitCombatCalculation OnBeingTargetedForEffects;

	// --- Incoming Event Handlers ---
	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;
	UFUNCTION()
	void HandleTurnStart(bool Emits = true);
	UFUNCTION()
	void HandleTurnEnd(bool Emits = true);
	UFUNCTION()
	void HandleAttacked(AUnit* Attacker, bool Emits = true);
	UFUNCTION()
	void HandleAttacks(AUnit* Target, bool Emits = true);
	UFUNCTION()
	void HandleMoved(const FTacMovementVisualData& MovementData);

	// --- Combat ---
	void HandleHit(const FDamageResult& Result, AUnit* Attacker, bool Emits = true);
	void ChangeUnitHP(int32 Delta, bool Emits = true);
	void ConsumeWard(EDamageSource Source, bool Emits = true);
	bool ApplyEffect(UBattleEffect* Effect, bool Emits = true);
	void NotifyEffectTriggered(UBattleEffect* Effect);
	void HandleDeath(bool Emits = true);

	// --- Queries ---
	FString GetLogName() const;
	const FGuid& GetUnitID() const { return UnitID; }
	FUnitCoreStats& GetStats() { return BaseStats; }
	const TArray<TObjectPtr<FWeapon>>& GetWeapons() const { return Weapons; }
	float GetMovementSpeed() const;
	UFUNCTION(BlueprintCallable)
	ETeamSide GetTeamSide() const { return GridMetadata.Team; }
	UFUNCTION(BlueprintCallable)
	void SetTeamSide(ETeamSide Side) { GridMetadata.Team = Side; }
	UFUNCTION(BlueprintCallable, Category = "Display")
	FUnitDisplayData GetDisplayData() const;
	const FUnitGridMetadata& GetGridMetadata() const { return GridMetadata; }
	UFUNCTION(BlueprintCallable, Category = "Grid")
	FUnitGridMetadata GetGridMetadataCopy() const { return GridMetadata; }
	UFUNCTION(BlueprintPure, Category = "Unit")
	UUnitDefinition* GetUnitDefinition() const { return UnitDefinition; }
	bool IsDead() const {return BaseStats.Status.IsDead();}
	bool CanAct() const { return BaseStats.Status.CanAct(); }

	// --- Components ---
	UFUNCTION(BlueprintPure, Category = "Components")
	UAbilityInventoryComponent* GetAbilityInventory() const { return AbilityInventory; }
	UFUNCTION(BlueprintPure, Category = "Components")
	UUnitVisualsComponent* GetVisualsComponent() { return VisualsComponent; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UUnitVisualsComponent> VisualsComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBattleEffectComponent> EffectManager;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilityInventoryComponent> AbilityInventory;

protected:
	// --- Core Data ---
	UPROPERTY(SaveGame)
	FGuid UnitID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
	TObjectPtr<UUnitDefinition> UnitDefinition;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FUnitCoreStats BaseStats;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<UWeapon>> Weapons;
private:
	// Grid positioning metadata (modified by UTacGridMovementService via friend access)
	FUnitGridMetadata GridMetadata;
	void NotifyOrientationChanged();  // Only UGridDataManager (friend) may call this.
	void InitializeWeapons(const UUnitDefinition* Definition);
};

typedef TArray<TObjectPtr<AUnit>> FUnitArray;
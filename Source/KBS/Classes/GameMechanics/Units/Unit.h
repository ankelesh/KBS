#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameMechanics/Units/Stats/UnitStats.h"
#include "UnitDisplayData.h"
#include "UnitGridMetadata.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "Unit.generated.h"

class UWeapon;
class UUnitDefinition;
class UBattleEffectComponent;
class UBattleEffect;
class UAbilityInventoryComponent;
class UUnitVisualsComponent;
class UTacGridMovementService;
struct FDamageResult;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitClicked, AUnit*, ClickedUnit, FKey, ButtonPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitAttacked, AUnit*, Victim, AUnit*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitDamaged, AUnit*, Victim, AUnit*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitAttacks, AUnit*, Attacker, AUnit*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitDied, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitTurnStart, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitTurnEnd, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitMoved, AUnit*, Unit);
UCLASS()
class KBS_API AUnit : public APawn
{
	GENERATED_BODY()
	friend class UTacGridMovementService;
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
	FOnUnitAttacks OnUnitAttacks;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitDied OnUnitDied;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitTurnStart OnUnitTurnStart;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitTurnEnd OnUnitTurnEnd;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnitMoved OnUnitMoved;

	// --- Incoming Event Handlers ---
	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;
	UFUNCTION()
	void HandleTurnStart();
	UFUNCTION()
	void HandleTurnEnd();
	UFUNCTION()
	void HandleAttacked(AUnit* Attacker);
	UFUNCTION()
	void HandleAttacks(AUnit* Target);
	UFUNCTION()
	void HandleMoved();

	// --- Combat ---
	void TakeHit(const FDamageResult& DamageResult);
	bool ApplyEffect(UBattleEffect* Effect);
	void SetDefending(bool bDefending);

	// --- Queries ---
	const FGuid& GetUnitID() const { return UnitID; }
	FUnitCoreStats& GetStats() { return BaseStats; }
	const TArray<TObjectPtr<UWeapon>>& GetWeapons() const { return Weapons; }
	float GetMovementSpeed() const;
	ETeamSide GetTeamSide() const { return TeamSide; }
	void SetTeamSide(ETeamSide Side) { TeamSide = Side; }
	virtual int32 GetCellSize() const { return 1; }
	virtual bool IsMultiCell() const { return false; }
	UFUNCTION(BlueprintCallable, Category = "Display")
	FUnitDisplayData GetDisplayData() const;
	const FUnitGridMetadata& GetGridMetadata() const { return GridMetadata; }
	UFUNCTION(BlueprintCallable, Category = "Grid")
	FUnitGridMetadata GetGridMetadataCopy() const { return GridMetadata; }

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	ETeamSide TeamSide = ETeamSide::Attacker;

private:
	// Grid positioning metadata (modified by UTacGridMovementService via friend access)
	FUnitGridMetadata GridMetadata;
};

typedef TArray<TObjectPtr<AUnit>> FUnitArray;
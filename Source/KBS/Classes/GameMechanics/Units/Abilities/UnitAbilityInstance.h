#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Units/Abilities/AbilityDisplayData.h"
#include "GameplayTypes/CombatTypes.h"
#include "UnitAbilityInstance.generated.h"

class UUnitAbilityDefinition;
struct FPreviewHitResult;

class AUnit;
class UTacGridSubsystem;
class UTacCombatSubsystem;
class UTacGridMovementService;
class UTacGridCombatSystem;
class UTacGridTargetingService;
class UTacTurnSubsystem;
class UTacAbilityExecutorService;
struct FAttackContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityAvailabilityChange, const UUnitAbilityInstance*, Ability, bool, Available);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityUsed, int32, ChargesLeft, bool, Available);
UCLASS(Abstract, Blueprintable)
class KBS_API UUnitAbilityInstance : public UObject
{
	GENERATED_BODY()
public:
	virtual void InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner);

	// Clean signature - no context bloat
	virtual bool Execute(FTacCoordinates TargetCell) { return true; };
	virtual bool CanExecute(FTacCoordinates TargetCell) const { return false; };
	virtual bool CanExecute() const { return false; };
	virtual bool RefreshAvailability() const;

	virtual ETargetReach GetTargeting() const;
	virtual bool IsPassive() const;
	virtual bool HasExplicitCharges() const { return false; }
	virtual TMap<FTacCoordinates, FPreviewHitResult> DamagePreview(FTacCoordinates TargetCell) const {return {};};
	virtual void Subscribe() {};
	virtual void Unsubscribe() {};
	virtual void AttackTriggerCleanup(FAttackContext& Context) {};
	virtual void HitTriggerCleanup(FHitInstance& Hit) {};
	
	UUnitAbilityDefinition* GetConfig() const { return Config; }
	int32 GetRemainingCharges() const { return RemainingCharges; }
	AUnit* GetOwner() const { return Owner; }
	void SetOwner(AUnit* NewOwner) { Owner = NewOwner; }
	virtual void ConsumeCharge();
	virtual void RestoreCharges();
	void BroadcastUsage() const;

	UFUNCTION(BlueprintCallable, Category = "Abilities|Display")
	virtual FAbilityDisplayData GetAbilityDisplayData() const;

	UPROPERTY(EditAnywhere, BlueprintAssignable, Category = "Abilities|Events")
	FOnAbilityAvailabilityChange OnAbilityAvailabilityChange;
	UPROPERTY(EditAnywhere, BlueprintAssignable, Category = "Abilities|Events")
	FOnAbilityUsed OnAbilityUsed;
protected:
	// Service getters - full chain encapsulated, returns nullptr on any failure
	UTacGridSubsystem* GetGridSubsystem() const;
	UTacGridMovementService* GetMovementService() const;
	UTacGridTargetingService* GetTargetingService() const;
	UTacCombatSubsystem* GetCombatSubsystem() const;
	UTacAbilityExecutorService* GetExecutorService() const;
	UTacTurnSubsystem* GetTurnSubsystem() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UUnitAbilityDefinition> Config;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	int32 RemainingCharges = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	TObjectPtr<AUnit> Owner;
};

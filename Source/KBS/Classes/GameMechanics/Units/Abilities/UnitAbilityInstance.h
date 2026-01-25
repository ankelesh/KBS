#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Units/Abilities/AbilityDisplayData.h"
#include "UnitAbilityInstance.generated.h"

class UUnitAbilityDefinition;
struct FPreviewHitResult;

class AUnit;
class UGridSubsystem;
class UTacCombatSubsystem;
class UTacGridMovementService;
class UTacGridCombatSystem;
class UTacGridTargetingService;

UCLASS(Abstract, Blueprintable)
class KBS_API UUnitAbilityInstance : public UObject
{
	GENERATED_BODY()
public:
	void InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner);

	// Clean signature - no context bloat
	virtual FAbilityResult TriggerAbility(AUnit* SourceUnit, FTacCoordinates TargetCell);
	virtual FAbilityResult ApplyAbilityEffect(AUnit* SourceUnit, FTacCoordinates TargetCell);
	virtual FAbilityValidation CanExecute(AUnit* SourceUnit, FTacCoordinates TargetCell) const;

	virtual ETargetReach GetTargeting() const;
	virtual bool IsPassive() const;
	virtual TMap<AUnit*, FPreviewHitResult> DamagePreview(AUnit* Source, const TArray<AUnit*>& Targets);
	virtual void Subscribe();
	virtual void Unsubscribe();
	virtual FAbilityResult CreateSuccessResult() const;
	virtual FAbilityResult CreateFailureResult(EAbilityFailureReason Reason, FText Message) const;

	UFUNCTION()
	virtual void HandleUnitAttacked(AUnit* Victim, AUnit* Attacker);
	UFUNCTION()
	virtual void HandleUnitDamaged(AUnit* Victim, AUnit* Attacker);
	UFUNCTION()
	virtual void HandleUnitAttacks(AUnit* Attacker, AUnit* Target);

	UUnitAbilityDefinition* GetConfig() const { return Config; }
	int32 GetRemainingCharges() const { return RemainingCharges; }
	AUnit* GetOwner() const { return Owner; }
	void ConsumeCharge();
	void RestoreCharges();

	UFUNCTION(BlueprintCallable, Category = "Abilities|Display")
	FAbilityDisplayData GetAbilityDisplayData() const;

protected:
	// Service getters - full chain encapsulated, returns nullptr on any failure
	UGridSubsystem* GetGridSubsystem(AUnit* Unit) const;
	UTacGridMovementService* GetMovementService(AUnit* Unit) const;
	UTacGridTargetingService* GetTargetingService(AUnit* Unit) const;
	UTacCombatSubsystem* GetCombatSubsystem(AUnit* Unit) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UUnitAbilityDefinition> Config;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	int32 RemainingCharges = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	TObjectPtr<AUnit> Owner;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	EAbilityTurnAction TurnAction = EAbilityTurnAction::EndTurn;
};

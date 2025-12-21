// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/AbilityBattleContext.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameMechanics/Units/Abilities/AbilityDisplayData.h"
#include "UnitAbilityInstance.generated.h"

class UUnitAbilityDefinition;
class AUnit;
struct FPreviewHitResult;

UCLASS(Abstract, Blueprintable)
class KBS_API UUnitAbilityInstance : public UObject
{
	GENERATED_BODY()

public:
	void InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner);

	virtual FAbilityResult TriggerAbility(const FAbilityBattleContext& Context);
	virtual FAbilityResult ApplyAbilityEffect(const FAbilityBattleContext& Context);
	virtual ETargetReach GetTargeting() const;
	virtual bool IsPassive() const;
	virtual TMap<AUnit*, FPreviewHitResult> DamagePreview(AUnit* Source, const TArray<AUnit*>& Targets);
	virtual void Subscribe();
	virtual void Unsubscribe();

	/**
	 * Validate ability can execute - check charges, targets, costs
	 */
	virtual FAbilityValidation CanExecute(const FAbilityBattleContext& Context) const;

	/**
	 * Create result object for successful execution
	 */
	virtual FAbilityResult CreateSuccessResult() const;

	/**
	 * Create result object for failed execution
	 */
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UUnitAbilityDefinition> Config;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	int32 RemainingCharges = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	TObjectPtr<AUnit> Owner;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	EAbilityTurnAction TurnAction = EAbilityTurnAction::EndTurn;
};

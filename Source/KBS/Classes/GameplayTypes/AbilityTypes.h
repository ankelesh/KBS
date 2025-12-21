// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilityTypes.generated.h"

class UBattleEffect;
class AUnit;

UENUM(BlueprintType)
enum class EAbilityTurnAction : uint8
{
	EndTurn UMETA(DisplayName = "End Turn"),
	FreeTurn UMETA(DisplayName = "Free Turn"),
	EndTurnDelayed UMETA(DisplayName = "End Turn Delayed"),
	RequireConfirm UMETA(DisplayName = "Require Confirm"),
	Wait UMETA(DisplayName = "Wait")
};

UENUM(BlueprintType)
enum class EAbilityFailureReason : uint8
{
	None UMETA(DisplayName = "None"),
	NoCharges UMETA(DisplayName = "No Charges"),
	InvalidTarget UMETA(DisplayName = "Invalid Target"),
	OutOfRange UMETA(DisplayName = "Out Of Range"),
	Interrupted UMETA(DisplayName = "Interrupted"),
	Custom UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct FAbilityValidation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly)
	EAbilityFailureReason FailureReason = EAbilityFailureReason::None;

	UPROPERTY(BlueprintReadOnly)
	FText FailureMessage;

	FAbilityValidation() = default;

	FAbilityValidation(bool bInIsValid, EAbilityFailureReason InReason = EAbilityFailureReason::None, FText InMessage = FText())
		: bIsValid(bInIsValid), FailureReason(InReason), FailureMessage(InMessage)
	{}

	static FAbilityValidation Success()
	{
		return FAbilityValidation(true);
	}

	static FAbilityValidation Failure(EAbilityFailureReason Reason, FText Message = FText())
	{
		return FAbilityValidation(false, Reason, Message);
	}
};

USTRUCT(BlueprintType)
struct FAbilityResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly)
	EAbilityTurnAction TurnAction = EAbilityTurnAction::EndTurn;

	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<UBattleEffect>> EffectsApplied;

	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<AUnit>> UnitsAffected;

	UPROPERTY(BlueprintReadOnly)
	EAbilityFailureReason FailureReason = EAbilityFailureReason::None;

	UPROPERTY(BlueprintReadOnly)
	FText FailureMessage;

	FAbilityResult() = default;

	FAbilityResult(bool bInSuccess, EAbilityTurnAction InTurnAction = EAbilityTurnAction::EndTurn)
		: bSuccess(bInSuccess), TurnAction(InTurnAction)
	{}

	static FAbilityResult Success(EAbilityTurnAction InTurnAction = EAbilityTurnAction::EndTurn)
	{
		return FAbilityResult(true, InTurnAction);
	}

	static FAbilityResult Failure(EAbilityFailureReason Reason, FText Message = FText())
	{
		FAbilityResult Result(false);
		Result.FailureReason = Reason;
		Result.FailureMessage = Message;
		return Result;
	}
};

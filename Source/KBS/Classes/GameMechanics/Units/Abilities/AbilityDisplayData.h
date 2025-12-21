// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilityDisplayData.generated.h"

/**
 * Lightweight display data for ability UI widgets
 * Encapsulates all information needed to render an ability slot
 */
USTRUCT(BlueprintType)
struct KBS_API FAbilityDisplayData
{
	GENERATED_BODY()

	/** Name of the ability */
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	FString AbilityName;

	/** Ability icon texture */
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	TObjectPtr<UTexture2D> Icon;

	/** Current remaining charges */
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	int32 RemainingCharges = 0;

	/** Maximum charges (-1 for unlimited) */
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	int32 MaxCharges = -1;

	/** Can this ability be executed this turn? (simplified check) */
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	bool bCanExecuteThisTurn = false;

	/** Tooltip description text */
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	FString Description;

	/** Targeting information string (e.g., "Any Enemy", "Area") */
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	FString TargetingInfo;

	/** If true, this slot is empty and should be collapsed */
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	bool bIsEmpty = true;
};

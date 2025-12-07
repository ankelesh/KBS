#pragma once

#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "UnitStats.generated.h"

USTRUCT(BlueprintType)
struct FUnitDefenseStats
{
	GENERATED_BODY()

	// One-time block per damage source
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	TSet<EDamageSource> Wards;

	// Immunity flags per damage source
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	TSet<EDamageSource> Immunities;

	// Percentage damage reduction per source (0.0-0.9, capped at 90%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	TMap<EDamageSource, float> Armour;

	// Flat damage reduction applied to all sources
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	int32 DamageReduction = 0;
};

USTRUCT(BlueprintType)
struct FUnitCoreStats
{
	GENERATED_BODY()

	// Health pool (float to avoid truncation on level-ups)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	float MaxHealth = 100.0f;

	// Initiative value (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	int32 Initiative = 50;

	// Accuracy as percentage (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	float Accuracy = 0.75f;

	// Defense characteristics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	FUnitDefenseStats Defense;
};

USTRUCT(BlueprintType)
struct FUnitProgressionData
{
	GENERATED_BODY()

	// Accumulated experience points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 TotalExperience = 0;

	// Current progression toward next level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 ExperienceToNextLevel = 0;

	// Static threshold for next level (per unit type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 NextLevelThreshold = 100;

	// Static tier identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 UnitTier = 1;

	// Current level within tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 LevelOnCurrentTier = 1;
};


#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "UnitStats.generated.h"
USTRUCT(BlueprintType)
struct FUnitDefenseStats
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	TSet<EDamageSource> Wards;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	TSet<EDamageSource> Immunities;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	TMap<EDamageSource, float> Armour;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	int32 DamageReduction = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	bool bIsDefending = false;
};
USTRUCT(BlueprintType)
struct FUnitCoreStats
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	float MaxHealth = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	int32 Initiative = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	float Accuracy = 0.75f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	FUnitDefenseStats Defense;
};
USTRUCT(BlueprintType)
struct FUnitProgressionData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 TotalExperience = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 ExperienceToNextLevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 NextLevelThreshold = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 UnitTier = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 LevelOnCurrentTier = 1;
};

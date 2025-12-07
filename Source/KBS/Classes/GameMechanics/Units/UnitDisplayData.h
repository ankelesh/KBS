#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "Weapons/WeaponDisplayData.h"
#include "UnitStats.h"
#include "UnitDisplayData.generated.h"

USTRUCT(BlueprintType)
struct KBS_API FUnitDisplayData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString UnitName;

	UPROPERTY(BlueprintReadWrite)
	float CurrentHealth;

	UPROPERTY(BlueprintReadWrite)
	float MaxHealth;

	UPROPERTY(BlueprintReadWrite)
	int32 Initiative;

	UPROPERTY(BlueprintReadWrite)
	float Accuracy;

	UPROPERTY(BlueprintReadWrite)
	int32 Level;

	UPROPERTY(BlueprintReadWrite)
	int32 Experience;

	UPROPERTY(BlueprintReadWrite)
	int32 ExperienceToNextLevel;

	// Portrait reference (texture or material)
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* PortraitTexture;

	// Active effects for display
	UPROPERTY(BlueprintReadWrite)
	FString ActiveEffectNames;

	UPROPERTY(BlueprintReadWrite)
	TArray<FWeaponDisplayData> Weapons;

	// Defense stats
	UPROPERTY(BlueprintReadWrite)
	FString Immunities;

	UPROPERTY(BlueprintReadWrite)
	FString Wards;

	UPROPERTY(BlueprintReadWrite)
	FString Armour; // "Physical: 50%, Fire: 30%"

	UPROPERTY(BlueprintReadWrite)
	int32 DamageReduction;

	UPROPERTY(BlueprintReadWrite)
	bool BelongsToAttackerTeam;
};

USTRUCT(BlueprintType)
struct KBS_API FUnitTurnQueueDisplay
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString UnitName;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentInitiative;

	UPROPERTY(BlueprintReadWrite)
	UTexture2D* PortraitTexture;

	UPROPERTY(BlueprintReadWrite)
	bool bIsActiveUnit;

	UPROPERTY(BlueprintReadWrite)
	bool BelongsToAttackerTeam;
};

// Forward declarations for conversion helpers
class UWeapon;
class UBattleEffect;
enum class EDamageSource : uint8;
enum class ETargetReach : uint8;
enum class ETeamSide : uint8;

// Conversion helper functions
FString DamageSourceToString(EDamageSource Source);
FString TargetReachToString(ETargetReach Reach);
FWeaponDisplayData ConvertWeapon(const UWeapon* Weapon);
TArray<FString> ConvertDamageSourceSet(const TSet<EDamageSource>& SourceSet);
TArray<FString> ConvertActiveEffects(const TArray<TObjectPtr<UBattleEffect>>& Effects);
FString ConvertArmourMap(const TMap<EDamageSource, float>& ArmourMap);

// Main conversion function
FUnitDisplayData BuildUnitDisplayData(
	const FString& UnitName,
	float CurrentHealth,
	const FUnitCoreStats& ModifiedStats,
	const FUnitProgressionData& Progression,
	UTexture2D* PortraitTexture,
	const TArray<TObjectPtr<UBattleEffect>>& ActiveEffects,
	const TArray<TObjectPtr<UWeapon>>& Weapons,
	ETeamSide TeamSide
);
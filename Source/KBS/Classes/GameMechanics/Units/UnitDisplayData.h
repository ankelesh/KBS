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
	float CurrentHealth = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	float MaxHealth = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	int32 Initiative = 0;
	UPROPERTY(BlueprintReadWrite)
	float Accuracy = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	int32 Level = 0;
	UPROPERTY(BlueprintReadWrite)
	int32 Experience = 0;
	UPROPERTY(BlueprintReadWrite)
	int32 ExperienceToNextLevel = 0;
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* PortraitTexture = nullptr;
	UPROPERTY(BlueprintReadWrite)
	FString ActiveEffectNames;
	UPROPERTY(BlueprintReadWrite)
	TArray<FWeaponDisplayData> Weapons;
	UPROPERTY(BlueprintReadWrite)
	FString Immunities;
	UPROPERTY(BlueprintReadWrite)
	FString Wards;
	UPROPERTY(BlueprintReadWrite)
	FString Armour;
	UPROPERTY(BlueprintReadWrite)
	int32 DamageReduction = 0;
	UPROPERTY(BlueprintReadWrite)
	bool bIsDefending = false;
	UPROPERTY(BlueprintReadWrite)
	bool BelongsToAttackerTeam = false;
};
USTRUCT(BlueprintType)
struct KBS_API FUnitTurnQueueDisplay
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	FString UnitName;
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentInitiative = 0;
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* PortraitTexture = nullptr;
	UPROPERTY(BlueprintReadWrite)
	bool bIsActiveUnit = false;
	UPROPERTY(BlueprintReadWrite)
	bool BelongsToAttackerTeam = false;
};
class UWeapon;
class UBattleEffect;
enum class EDamageSource : uint8;
enum class ETargetReach : uint8;
enum class ETeamSide : uint8;
FString DamageSourceToString(EDamageSource Source);
FString TargetReachToString(ETargetReach Reach);
FWeaponDisplayData ConvertWeapon(const UWeapon* Weapon);
TArray<FString> ConvertDamageSourceSet(const TSet<EDamageSource>& SourceSet);
TArray<FString> ConvertActiveEffects(const TArray<TObjectPtr<UBattleEffect>>& Effects);
FString ConvertArmourMap(const TMap<EDamageSource, float>& ArmourMap);
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

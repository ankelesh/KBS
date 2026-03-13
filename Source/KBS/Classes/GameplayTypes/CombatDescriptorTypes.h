#pragma once
#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "GameplayTypes/DamageTypes.h"
#include "CombatDescriptorTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatDescriptorDesignation : uint8
{
	AllPurpose  UMETA(DisplayName = "All Purpose"),
	AutoAttacks UMETA(DisplayName = "Auto Attacks Only"),
	Spells      UMETA(DisplayName = "Spells Only"),
};

UENUM(BlueprintType)
enum class EWardApplicationPolicy : uint8
{
	None UMETA(DisplayName = "None"),
	Give UMETA(DisplayName = "Give"),
	Take UMETA(DisplayName = "Take"),
};

UENUM(BlueprintType)
enum class EDispelPolarityFilter : uint8
{
	None        UMETA(DisplayName = "None"),
	Any         UMETA(DisplayName = "Any"),
	Positive    UMETA(DisplayName = "Positive"),
	Negative    UMETA(DisplayName = "Negative"),
	Neutral     UMETA(DisplayName = "Neutral"),
	NonPositive UMETA(DisplayName = "Non-Positive"),
	NonNegative UMETA(DisplayName = "Non-Negative"),
};

USTRUCT(BlueprintType)
struct FDescriptorSideEffects
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wards")
	TSet<EDamageSource> WardSources;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wards")
	EWardApplicationPolicy WardPolicy = EWardApplicationPolicy::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispel")
	TArray<FGameplayTag> DispelTags;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispel")
	EDispelPolarityFilter DispelPolarity = EDispelPolarityFilter::Any;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stance")
	bool bRemovesDefensiveStance = false;

	bool IsActive() const
	{
		return WardPolicy != EWardApplicationPolicy::None
			|| DispelPolarity != EDispelPolarityFilter::None
			|| bRemovesDefensiveStance;
	}
};

UENUM(BlueprintType)
enum class EMagnitudePolicy : uint8
{
	// Runs damage calculation and application phase
	Damage UMETA(DisplayName = "Damage"),
	// Runs heal calculation and application phase
	Heal   UMETA(DisplayName = "Heal"),
	// Skips damage calculation and application phase entirely
	None   UMETA(DisplayName = "None"),
};

#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "UnitStatDelta.generated.h"

// Ultimate container for any stat-based alteration to a unit.
// Core/Defense fields are applied by the battle effect system.
// Combat fields are applied by the weapon/spell descriptor system (different pathway).
USTRUCT(BlueprintType)
struct FUnitStatDelta
{
	GENERATED_BODY()

	// --- Core stats ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (ClampMin = "-1000", ClampMax = "1000"))
	int32 MaxHealth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (ClampMin = "-100", ClampMax = "100"))
	int32 Initiative = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (ClampMin = "-100", ClampMax = "100"))
	int32 Accuracy = 0;

	// --- Defense stats ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	TSet<EDamageSource> ImmunitiesToGrant;

	// Flat per-source modifier; negative values reduce armour
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	TMap<EDamageSource, int32> ArmourDeltas;

	// --- Combat descriptor stats ---
	// Applied via UCombatDescriptor::ModifyMagnitude(..., bIsFlat=true)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "-500", ClampMax = "500"))
	int32 MagnitudeFlat = 0;

	// Applied via UCombatDescriptor::ModifyMagnitude(..., bIsFlat=false)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "-100", ClampMax = "100"))
	int32 MagnitudeMultiplier = 0;

	// Applied via UCombatDescriptor::ModifySource
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSet<EDamageSource> DamageSourcesToAdd;

	// Overrides bGuaranteedHit on the descriptor when true
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bGrantsGuaranteedHit = false;

	bool IsEmpty() const
	{
		return MaxHealth == 0 && Initiative == 0 && Accuracy == 0
			&& ImmunitiesToGrant.IsEmpty() && ArmourDeltas.IsEmpty()
			&& MagnitudeFlat == 0 && MagnitudeMultiplier == 0
			&& DamageSourcesToAdd.IsEmpty() && !bGrantsGuaranteedHit;
	}

	void Reset()
	{
		MaxHealth = 0; Initiative = 0; Accuracy = 0;
		ImmunitiesToGrant.Empty(); ArmourDeltas.Empty();
		MagnitudeFlat = 0; MagnitudeMultiplier = 0;
		DamageSourcesToAdd.Empty(); bGrantsGuaranteedHit = false;
	}
};

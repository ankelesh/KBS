#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "UnitInstantChange.generated.h"

// One-shot, irreversible change to a unit's current state (contrast FUnitStatDelta: reversible modifiers).
// Applied through AUnit::ApplyInstantChange.
USTRUCT(BlueprintType)
struct FUnitInstantChange
{
	GENERATED_BODY()

	// Wards stay until consumed by a matching hit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	TSet<EDamageSource> WardsToGrant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0"))
	int32 HealFlat = 0;

	// Percent of maximum health, added on top of HealFlat.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0", ClampMax = "100"))
	int32 HealPercentOfMax = 0;

	bool IsEmpty() const { return WardsToGrant.IsEmpty() && HealFlat == 0 && HealPercentOfMax == 0; }
};

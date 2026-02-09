#pragma once
#include "CoreMinimal.h"
#include "BattleEffectDataAsset.h"
#include "GameplayTypes/DamageTypes.h"
#include "StatModBattleEffectDataAsset.generated.h"

UCLASS(BlueprintType)
class KBS_API UStatModBattleEffectDataAsset : public UBattleEffectDataAsset
{
	GENERATED_BODY()
public:
	UStatModBattleEffectDataAsset()
	{
		DamageSource = EDamageSource::Life;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	int32 Duration = 1;

	// Stat Modifiers (0 = no modification)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Modifiers", meta = (ClampMin = "-1000", ClampMax = "1000"))
	int32 MaxHealthModifier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Modifiers", meta = (ClampMin = "-100", ClampMax = "100"))
	int32 InitiativeModifier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Modifiers", meta = (ClampMin = "-100", ClampMax = "100"))
	int32 AccuracyModifier = 0;

	// Defense Modifiers
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense Modifiers")
	TSet<EDamageSource> ImmunitiesToGrant;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense Modifiers", meta = (ClampMin = "-90", ClampMax = "90"))
	TMap<EDamageSource, int32> ArmourModifiers;
};

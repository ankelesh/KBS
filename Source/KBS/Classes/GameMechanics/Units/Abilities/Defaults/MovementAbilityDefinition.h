#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameplayTypes/TargetingDescriptor.h"
#include "MovementAbilityDefinition.generated.h"

UCLASS(BlueprintType)
class KBS_API UMovementAbilityDefinition : public UUnitAbilityDefinition
{
	GENERATED_BODY()
public:
	UMovementAbilityDefinition();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	FTargetingDescriptor GroundTargeting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	FTargetingDescriptor AirTargeting;

	// True = animated movement along path; False = instant teleport
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsAnimated = true;
};

#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "ReloadAbilityDefinition.generated.h"

UCLASS(BlueprintType)
class KBS_API UReloadAbilityDefinition : public UUnitAbilityDefinition
{
	GENERATED_BODY()
public:
	// Charge refilled to its cap by this ability; the owner needs a UUnitChargeComponent tracking it.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reload")
	FGameplayTag ChargeTag;
};

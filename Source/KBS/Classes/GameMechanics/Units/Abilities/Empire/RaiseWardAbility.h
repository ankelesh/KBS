#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/Defaults/DefendAbility.h"
#include "RaiseWardAbility.generated.h"

/**
 * Defend that also grants the owner one ward against a random elemental or Death source it does not already ward.
 * Nothing is granted when the owner already wards every source.
 */
UCLASS()
class KBS_API URaiseWardAbility : public UDefendAbility
{
	GENERATED_BODY()
public:
	virtual FAbilityExecutionResult Execute(FTacCoordinates TargetCell) override;
};

#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "FleeAbilityDefinition.generated.h"

UENUM(BlueprintType)
enum class EFleeTiming : uint8
{
	// The owner leaves when its next turn starts
	NextTurnStart UMETA(DisplayName = "Next Turn Start"),
	// The owner leaves when its current turn ends
	ThisTurnEnd UMETA(DisplayName = "This Turn End"),
	// The owner leaves as soon as the situation allows: an activated ability resolves it as ThisTurnEnd
	Instant UMETA(DisplayName = "Instant"),
};

UENUM(BlueprintType)
enum class EDepartureMode : uint8
{
	// Marks the owner Fleeing and leaves for good: effects cleared, passives unregistered
	Flee UMETA(DisplayName = "Flee"),
	// Leaves the field without the Fleeing status, so the unit can return later
	Withdraw UMETA(DisplayName = "Withdraw"),
};

UCLASS(BlueprintType)
class KBS_API UFleeAbilityDefinition : public UUnitAbilityDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flee")
	EFleeTiming Timing = EFleeTiming::NextTurnStart;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flee")
	EDepartureMode DepartureMode = EDepartureMode::Flee;
};

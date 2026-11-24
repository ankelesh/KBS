#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Combat/CombatDescriptorData.h"
#include "ProtectEveryoneAbility.generated.h"

UCLASS(BlueprintType)
class KBS_API UProtectEveryoneAbilityDefinition : public UUnitAbilityDefinition
{
	GENERATED_BODY()
public:
	UProtectEveryoneAbilityDefinition();

	// Applied to every living ally except the owner.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Protect")
	TArray<FDescriptorEffectConfig> AllyEffects;
};

/**
 * Ultimate: available once the owner's action history sequence is complete.
 * Applies AllyEffects to all allies, puts the owner in a defensive stance, consumes the sequence and ends the turn.
 */
UCLASS(Blueprintable)
class KBS_API UProtectEveryoneAbility : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual FAbilityExecutionResult Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override { return CanExecute(); }
	virtual bool CanExecute() const override;
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;

private:
	UFUNCTION()
	void HandleSequenceCompleted();
};

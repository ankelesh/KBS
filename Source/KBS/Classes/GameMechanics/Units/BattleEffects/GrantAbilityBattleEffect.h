#pragma once
#include "CoreMinimal.h"
#include "BattleEffect.h"
#include "GrantAbilityBattleEffect.generated.h"

class UUnitAbility;
class UUnitAbilityDefinition;

UCLASS(BlueprintType)
class KBS_API UGrantAbilityBattleEffectDataAsset : public UBattleEffectDataAsset
{
	GENERATED_BODY()
public:
	// Must be a passive ability definition.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UUnitAbilityDefinition> GrantedAbility;
};

/** Hosts a passive ability for the effect's lifetime: the ability supplies the behavior, the effect supplies duration and visibility. */
UCLASS(Blueprintable)
class KBS_API UGrantAbilityBattleEffect : public UBattleEffect
{
	GENERATED_BODY()
public:
	virtual void Initialize(UBattleEffectDataAsset* InConfig) override;
	virtual void OnApplied() override;
	virtual void OnRemoved() override;
	virtual void OnTurnEnd() override { DecrementDuration(); }

private:
	UPROPERTY()
	TObjectPtr<UUnitAbility> GrantedAbility;
};

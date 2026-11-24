#pragma once
#include "CoreMinimal.h"
#include "BattleEffect.h"
#include "DamageReductionBattleEffect.generated.h"

struct FCombatContext;
struct FHitInstance;
struct FDamageResult;

UCLASS(BlueprintType)
class KBS_API UDamageReductionBattleEffectDataAsset : public UBattleEffectDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = (ClampMin = "0", ClampMax = "100"))
	int32 ReductionPercent = 50;
};

/** Reduces incoming damage (not heals) dealt to the owner by a percentage, for its duration. */
UCLASS(Blueprintable)
class KBS_API UDamageReductionBattleEffect : public UBattleEffect
{
	GENERATED_BODY()
public:
	virtual void Initialize(UBattleEffectDataAsset* InConfig) override;
	virtual void OnApplied() override;
	virtual void OnRemoved() override;
	virtual void OnTurnEnd() override { DecrementDuration(); }

private:
	void OnDamageApplied(FCombatContext& Context, FHitInstance& Hit, FDamageResult& Result);

	FDelegateHandle HookHandle;
};

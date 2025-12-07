#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/DamageTypes.h"
#include "BattleEffectDataAsset.h"
#include "BattleEffect.generated.h"

class AUnit;

UCLASS(Abstract, Blueprintable)
class KBS_API UBattleEffect : public UObject
{
	GENERATED_BODY()

public:
	void InitializeFromDataAsset(UBattleEffectDataAsset* InConfig);

	virtual void OnTurnStart(AUnit* Owner) {}
	virtual void OnTurnEnd(AUnit* Owner) {}
	virtual void OnUnitAttacked(AUnit* Owner, AUnit* Attacker) {}
	virtual void OnUnitAttacks(AUnit* Owner, AUnit* Target) {}
	virtual void OnUnitMoved(AUnit* Owner) {}
	virtual void OnUnitDied(AUnit* Owner) {}
	virtual void OnApplied(AUnit* Owner) {}
	virtual void OnRemoved(AUnit* Owner) {}

	EDamageSource GetDamageSource() const;
	EEffectTarget GetEffectTarget() const;
	FText GetEffectName() const { return Config->Name; }
	int32 GetRemainingTurns() const { return RemainingTurns; }
	void DecrementTurns() { if (RemainingTurns > 0) RemainingTurns--; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UBattleEffectDataAsset> Config;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	int32 RemainingTurns = 0;
};

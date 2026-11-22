#pragma once
#include "CoreMinimal.h"
#include "BattleEffect.h"
#include "StatusBattleEffect.generated.h"

class UStatusBattleEffectDataAsset;

UCLASS(Blueprintable)
class KBS_API UStatusBattleEffect : public UBattleEffect
{
	GENERATED_BODY()
public:
	virtual void Initialize(UBattleEffectDataAsset* InConfig) override;
	virtual void OnApplied() override;
	virtual void OnRemoved() override;
	virtual void OnTurnEnd() override;

private:
	const UStatusBattleEffectDataAsset& GetStatusConfig() const;
};

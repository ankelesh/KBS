#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/DamageTypes.h"
#include "BattleEffect.generated.h"

class AUnit;

UENUM(BlueprintType)
enum class EEffectTarget : uint8
{
	Enemy UMETA(DisplayName = "Enemy"),
	Self UMETA(DisplayName = "Self")
};

UCLASS(Abstract, Blueprintable)
class KBS_API UBattleEffect : public UObject
{
	GENERATED_BODY()

public:
	virtual void ApplyEffect(AUnit* Target) PURE_VIRTUAL(UBattleEffect::ApplyEffect, );

	EDamageSource GetDamageSource() const { return DamageSource; }
	EEffectTarget GetEffectTarget() const { return EffectTarget; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	EDamageSource DamageSource = EDamageSource::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	EEffectTarget EffectTarget = EEffectTarget::Enemy;
};

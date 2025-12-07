// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "UnitAutoAttackAbility.generated.h"

UCLASS(Blueprintable)
class KBS_API UUnitAutoAttackAbility : public UUnitAbilityInstance
{
	GENERATED_BODY()

public:
	virtual ETargetReach GetTargeting() const override;
	virtual TMap<AUnit*, FPreviewHitResult> DamagePreview(AUnit* Source, const TArray<AUnit*>& Targets) override;
	virtual FAbilityResult ApplyAbilityEffect(const FAbilityBattleContext& Context) override;
};

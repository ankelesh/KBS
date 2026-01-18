// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TacAbilityExecutorService.generated.h"

namespace Tactical {
	UCLASS()
		class KBS_API UTacAbilityExecutorService : public UObject
	{
		GENERATED_BODY()

	public:
		void Initialize(ATacBattleGrid* InGrid);
		FAbilityValidation ValidateAbility(UUnitAbilityInstance* Ability, const FAbilityBattleContext& Context) const;
		FAbilityResult ExecuteAbility(UUnitAbilityInstance* Ability, const FAbilityBattleContext& Context);
		FAbilityBattleContext BuildContext(AUnit* SourceUnit, const TArray<AUnit*>& Targets) const;
		FAbilityBattleContext BuildContext(AUnit* SourceUnit, const TArray<AUnit*>& Targets, FIntPoint ClickedCell, uint8 ClickedLayer) const;
		void ResolveResult(const FAbilityResult& Result);
	private:
		UPROPERTY()
		TObjectPtr<ATacBattleGrid> Grid;
	};
}; // namespace Tactical
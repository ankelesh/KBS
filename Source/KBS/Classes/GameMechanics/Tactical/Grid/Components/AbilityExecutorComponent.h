#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameplayTypes/AbilityBattleContext.h"
#include "AbilityExecutorComponent.generated.h"
class ATacBattleGrid;
class UUnitAbilityInstance;
class AUnit;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KBS_API UAbilityExecutorComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UAbilityExecutorComponent();
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

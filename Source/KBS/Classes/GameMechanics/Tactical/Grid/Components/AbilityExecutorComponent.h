#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameplayTypes/AbilityBattleContext.h"
#include "AbilityExecutorComponent.generated.h"
class ATacBattleGrid;
class UUnitAbilityInstance;
class UPresentationTrackerComponent;
class AUnit;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KBS_API UAbilityExecutorComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UAbilityExecutorComponent();
	void Initialize(ATacBattleGrid* InGrid, UPresentationTrackerComponent* InPresentationTracker);
	FAbilityValidation ValidateAbility(UUnitAbilityInstance* Ability, const FAbilityBattleContext& Context) const;
	FAbilityResult ExecuteAbility(UUnitAbilityInstance* Ability, const FAbilityBattleContext& Context);
	FAbilityBattleContext BuildContext(AUnit* SourceUnit, const TArray<AUnit*>& Targets) const;
	void ResolveResult(const FAbilityResult& Result);
private:
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;
	UPROPERTY()
	TObjectPtr<UPresentationTrackerComponent> PresentationTracker;
};

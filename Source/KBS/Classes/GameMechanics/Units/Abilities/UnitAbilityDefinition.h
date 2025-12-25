#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTypes/DamageTypes.h"
#include "UnitAbilityDefinition.generated.h"
class UBattleEffect;
class UUnitAbilityInstance;
UCLASS(BlueprintType)
class KBS_API UUnitAbilityDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<UUnitAbilityInstance> AbilityClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FString AbilityName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	int32 MaxCharges = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	ETargetReach Targeting = ETargetReach::AnyEnemy;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	TArray<TSubclassOf<UBattleEffect>> BattleEffects;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	bool bIsPassive = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spellbook")
	bool bIsSpellbookAbility = false;
};

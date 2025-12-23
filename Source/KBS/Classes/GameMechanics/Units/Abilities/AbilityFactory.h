#pragma once
#include "CoreMinimal.h"
#include "AbilityFactory.generated.h"
class UUnitAbilityDefinition;
class UUnitAbilityInstance;
class AUnit;
UCLASS()
class KBS_API UAbilityFactory : public UObject
{
	GENERATED_BODY()
public:
	static UUnitAbilityInstance* CreateAbilityFromDefinition(UUnitAbilityDefinition* Definition, AUnit* Owner);
};

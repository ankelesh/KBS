#pragma once
#include "CoreMinimal.h"
#include "AbilityFactory.generated.h"
class UUnitAbilityDefinition;
class UUnitAbility;
class AUnit;
UCLASS()
class KBS_API UAbilityFactory : public UObject
{
	GENERATED_BODY()
public:
	static UUnitAbility* CreateAbilityFromDefinition(UUnitAbilityDefinition* Definition, AUnit* Owner);
};

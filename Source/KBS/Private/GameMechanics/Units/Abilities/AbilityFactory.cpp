#include "GameMechanics/Units/Abilities/AbilityFactory.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Unit.h"
UUnitAbilityInstance* UAbilityFactory::CreateAbilityFromDefinition(UUnitAbilityDefinition* Definition, AUnit* Owner)
{
	if (!Definition || !Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityFactory: Cannot create ability - Definition or Owner is null"));
		return nullptr;
	}
	if (!Definition->AbilityClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityFactory: AbilityClass not set in definition '%s'"), *Definition->GetName());
		return nullptr;
	}
	UUnitAbilityInstance* NewAbility = NewObject<UUnitAbilityInstance>(Owner, Definition->AbilityClass);
	if (!NewAbility)
	{
		UE_LOG(LogTemp, Error, TEXT("AbilityFactory: Failed to instantiate ability class '%s'"), *Definition->AbilityClass->GetName());
		return nullptr;
	}
	NewAbility->InitializeFromDefinition(Definition, Owner);
	return NewAbility;
}

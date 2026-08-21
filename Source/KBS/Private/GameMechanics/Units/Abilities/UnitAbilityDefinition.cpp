#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"

EDataValidationResult UUnitAbilityDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// Abilities with combat effects should declare a presentation manifest so projectiles,
	// delivery topology, etc. are properly authored. Absent is legal but designers should know.
	if (!BattleEffects.IsEmpty() && Presentation.IsNull())
	{
		Context.AddWarning(FText::Format(
			NSLOCTEXT("AbilityValidation", "MissingPresentation",
				"Ability '{0}' has BattleEffects but no Presentation manifest assigned. "
				"Combat-step visuals (delivery, projectile) will be skipped."),
			FText::FromString(GetName())));
	}

	return Result;
}

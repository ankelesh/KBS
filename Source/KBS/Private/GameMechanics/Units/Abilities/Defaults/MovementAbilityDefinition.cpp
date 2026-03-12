#include "GameMechanics/Units/Abilities/Defaults/MovementAbilityDefinition.h"

UMovementAbilityDefinition::UMovementAbilityDefinition()
{
	GroundTargeting.Strategy = ETargetingStrategy::Movement;
	GroundTargeting.MovementPattern = EMovementPattern::Orthogonal;
	GroundTargeting.MovementLayer = EMovementLayer::Ground;

	AirTargeting.Strategy = ETargetingStrategy::Movement;
	AirTargeting.MovementPattern = EMovementPattern::AnyToAny;
	AirTargeting.MovementLayer = EMovementLayer::Air;
}

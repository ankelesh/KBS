#include "Presentation/Tactical/Converters/UnitMoveOffFieldPayloadConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/TacticalMovementConstants.h"
#include "Presentation/Core/Actions/MovePresentationAction.h"

UMovePresentationAction* TacticalLogConverters::ConvertUnitMoveOffFieldPayload(const FUnitMoveOffFieldPayload& Payload, const TMap<FGuid, AUnit*>& UnitLookup, UTacGridSubsystem* GridSubsystem)
{
	AUnit* const* FoundUnit = UnitLookup.Find(Payload.UnitId);
	checkf(FoundUnit, TEXT("Unit move-off-field payload references unit %s not found on grid"), *Payload.UnitId.ToString());
	AUnit* Unit = *FoundUnit;

	const float ExitYaw = (Payload.TeamSide == ETeamSide::Attacker)
		? FTacMovementConstants::DefenderDefaultYaw
		: FTacMovementConstants::AttackerDefaultYaw;
	const FVector ExitDirection = FRotator(0.f, ExitYaw, 0.f).Vector();

	const FVector Start = GridSubsystem->GetCellWorldLocation(Payload.LastFieldCoords);
	const FVector End = Start + ExitDirection * FTacMovementConstants::OffFieldExitDistance;
	const float Duration = FVector::Dist(Start, End) / Unit->GetMovementSpeed();

	UMovePresentationAction* Action = NewObject<UMovePresentationAction>();
	Action->Context.Actor = Unit;
	Action->Context.Path.Add(FPresentationMoveSegment(Start, End, Duration, ExitDirection.Rotation()));
	return Action;
}

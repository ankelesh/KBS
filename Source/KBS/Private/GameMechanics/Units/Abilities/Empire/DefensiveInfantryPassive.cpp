#include "GameMechanics/Units/Abilities/Empire/DefensiveInfantryPassive.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"

void UDefensiveInfantryPassive::OnBattleStart()
{
	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(LogSubsystem);
	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::Ability, ETacLogEventOrigin::Triggered,
	                                        Owner->GetUnitID(), FGuid());

	Owner->GetStats().Status.SetDefending();

	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogStatusChangeStep>(
		FTacLogStatusChangeStep::Make(Owner->GetUnitID(), EUnitStatus::Defending, true)));
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));
}

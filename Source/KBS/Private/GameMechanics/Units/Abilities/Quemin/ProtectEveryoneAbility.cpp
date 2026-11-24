#include "GameMechanics/Units/Abilities/Quemin/ProtectEveryoneAbility.h"
#include "GameMechanics/Units/Components/UnitActionHistoryComponent.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"

UProtectEveryoneAbilityDefinition::UProtectEveryoneAbilityDefinition()
{
	AbilityClass = UProtectEveryoneAbility::StaticClass();
	Targeting = ETargetReach::Self;
	DefaultReleasePolicy = EAbilityTurnReleasePolicy::Released;
}

FAbilityExecutionResult UProtectEveryoneAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(LogSubsystem);
	check(TargetingService);

	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::Ability, ETacLogEventOrigin::Initiated,
	                                        Owner->GetUnitID(), FGuid());

	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Command           = TargetCell;

	const TArray<FDescriptorEffectConfig>& AllyEffects = CastChecked<UProtectEveryoneAbilityDefinition>(Config)->AllyEffects;
	for (AUnit* Ally : TargetingService->GetValidTargetUnits(Owner, ETargetReach::AllFriendlies))
		ApplyConfiguredEffects(Ally, AllyEffects, Payload.Steps);

	Owner->GetStats().Status.SetDefending();
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogStatusChangeStep>(
		FTacLogStatusChangeStep::Make(Owner->GetUnitID(), EUnitStatus::Defending, true)));
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));

	Owner->GetActionHistory().ResetSequence();
	ConsumeUse();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UProtectEveryoneAbility::CanExecute() const
{
	return Owner && RemainingUses > 0 && OwnerCanAct() && CanActByContext()
		&& Owner->GetActionHistory().IsSequenceComplete();
}

void UProtectEveryoneAbility::Subscribe()
{
	Owner->GetActionHistory().OnSequenceCompleted.AddDynamic(this, &UProtectEveryoneAbility::HandleSequenceCompleted);
}

void UProtectEveryoneAbility::Unsubscribe()
{
	Owner->GetActionHistory().OnSequenceCompleted.RemoveDynamic(this, &UProtectEveryoneAbility::HandleSequenceCompleted);
}

void UProtectEveryoneAbility::HandleSequenceCompleted()
{
	RefreshAvailability();
}

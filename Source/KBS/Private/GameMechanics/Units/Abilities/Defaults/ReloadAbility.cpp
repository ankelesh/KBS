#include "GameMechanics/Units/Abilities/Defaults/ReloadAbility.h"
#include "GameMechanics/Units/Abilities/Defaults/ReloadAbilityDefinition.h"
#include "GameMechanics/Units/Components/UnitChargeComponent.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"

FAbilityExecutionResult UReloadAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(LogSubsystem);

	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::Ability, ETacLogEventOrigin::Initiated,
	                                        Owner->GetUnitID(), FGuid());

	UUnitChargeComponent& Charges = Owner->GetCharges();
	const FGameplayTag ChargeTag = GetReloadConfig().ChargeTag;
	const int32 ChargeBefore = Charges.GetCharge(ChargeTag);
	Charges.RefreshCharge(ChargeTag);

	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Command           = TargetCell;
	AppendChargeStep(Payload.Steps, ChargeTag, Charges.GetCharge(ChargeTag) - ChargeBefore);
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));

	ConsumeUse();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UReloadAbility::CanExecute(FTacCoordinates TargetCell) const
{
	return CanExecute();
}

bool UReloadAbility::CanExecute() const
{
	return Owner && RemainingUses > 0 && OwnerCanAct() && CanActByContext()
		&& !Owner->GetCharges().IsFull(GetReloadConfig().ChargeTag);
}

const UReloadAbilityDefinition& UReloadAbility::GetReloadConfig() const
{
	const UReloadAbilityDefinition* ReloadConfig = Cast<UReloadAbilityDefinition>(Config);
	checkf(ReloadConfig && ReloadConfig->ChargeTag.IsValid(),
	       TEXT("UReloadAbility requires a UReloadAbilityDefinition with ChargeTag"));
	return *ReloadConfig;
}

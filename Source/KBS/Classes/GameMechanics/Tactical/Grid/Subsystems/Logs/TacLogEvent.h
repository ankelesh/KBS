#pragma once
#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "GameplayTypes/LogTypesLibrary.h"
#include "TacLogEvent.generated.h"

USTRUCT(BlueprintType)
struct KBS_API FTacLogEvent
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid EventId;

	UPROPERTY()
	FGuid ParentId;

	UPROPERTY()
	int32 SequencePosition = 0;

	UPROPERTY()
	ETacLogEventType Type = ETacLogEventType::Ability;

	UPROPERTY()
	ETacLogEventOrigin Origin = ETacLogEventOrigin::Initiated;

	UPROPERTY()
	ETacLogEventState State = ETacLogEventState::Open;

	UPROPERTY()
	FGuid InstigatorId;

	UPROPERTY()
	TInstancedStruct<FTacLogPayload> Payload;

	UPROPERTY()
	int32 RoundNumber = 0;

	UPROPERTY()
	int32 TurnNumber = 0;
};

#pragma once
#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "GameplayTypes/LogTypesLibrary.h"
#include "GameplayTypes/TeamConstants.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "TacLogPayloads.generated.h"

USTRUCT(BlueprintType)
struct KBS_API FAbilityUsePayload : public FTacLogPayload
{
	GENERATED_BODY()

	UPROPERTY()
	FPrimaryAssetId AbilityAssetId;

	UPROPERTY()
	FGuid AbilityInstanceId;

	UPROPERTY()
	TArray<TInstancedStruct<FTacLogStepBase>> Steps;

	UPROPERTY()
	FTacCoordinates Command;
};

USTRUCT(BlueprintType)
struct KBS_API FTacEffectPayload : public FTacLogPayload
{
	GENERATED_BODY()

	UPROPERTY()
	FPrimaryAssetId EffectAssetId;

	UPROPERTY()
	FGuid EffectInstanceId;

	UPROPERTY()
	FGuid OwnerUnitId;

	UPROPERTY()
	FGuid SourceId;

	UPROPERTY()
	TArray<TInstancedStruct<FTacLogStepBase>> Steps;
};

USTRUCT(BlueprintType)
struct KBS_API FEffectEndPayload : public FTacLogPayload
{
	GENERATED_BODY()

	UPROPERTY()
	FPrimaryAssetId EffectAssetId;

	UPROPERTY()
	FGuid EffectInstanceId;

	UPROPERTY()
	FGuid OwnerUnitId;

	UPROPERTY()
	EEffectRemovalReason RemovalReason = EEffectRemovalReason::Expired;
};

USTRUCT(BlueprintType)
struct KBS_API FTurnChangePayload : public FTacLogPayload
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid OldTurnOwner;

	UPROPERTY()
	FGuid NewTurnOwner;

	UPROPERTY()
	ETeamSide NewTurnTeam;

	UPROPERTY()
	ETurnChangeKind ChangeKind;
};

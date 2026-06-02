#pragma once
#include "CoreMinimal.h"
#include "LogTypesLibrary.generated.h"

UENUM(BlueprintType)
enum class ETacLogEventType : uint8
{
	Ability          UMETA(DisplayName = "Ability"),
	EffectActivation UMETA(DisplayName = "Effect Activation"),
	EffectEnd        UMETA(DisplayName = "Effect End"),
	TurnChange       UMETA(DisplayName = "Turn Change"),
};

UENUM(BlueprintType)
enum class ETacLogEventOrigin : uint8
{
	Initiated  UMETA(DisplayName = "Initiated"),
	Reaction   UMETA(DisplayName = "Reaction"),
	Triggered  UMETA(DisplayName = "Triggered"),
	Progressed UMETA(DisplayName = "Progressed"),
};

UENUM(BlueprintType)
enum class ETacLogEventState : uint8
{
	Open   UMETA(DisplayName = "Open"),
	Closed UMETA(DisplayName = "Closed"),
};

UENUM(BlueprintType)
enum class ETurnChangeKind : uint8
{
	Turn  UMETA(DisplayName = "Turn"),
	Round UMETA(DisplayName = "Round"),
};

UENUM(BlueprintType)
enum class EEffectRemovalReason : uint8
{
	Expired   UMETA(DisplayName = "Expired"),    // duration ran to zero
	Dispelled UMETA(DisplayName = "Dispelled"),  // actively removed
	Replaced  UMETA(DisplayName = "Replaced"),   // wiped by stack policy replacement
	OwnerDied UMETA(DisplayName = "Owner Died"), // unit carrying the effect died
};

UENUM(BlueprintType)
enum class EEffectApplicationOutcome : uint8
{
	Applied    UMETA(DisplayName = "Applied"),     // new instance, no prior with same stacking id
	Refreshed  UMETA(DisplayName = "Refreshed"),   // existing instance duration extended
	Replaced   UMETA(DisplayName = "Replaced"),    // new instance replaced old
	Stacked    UMETA(DisplayName = "Stacked"),     // new instance added alongside existing
	Rejected   UMETA(DisplayName = "Rejected"),    // blocked by stacking policy or stack limit
	RollMissed UMETA(DisplayName = "RollMissed"),  // accuracy roll failed before application
	Immune     UMETA(DisplayName = "Immune"),      // target immune to this effect
	Warded     UMETA(DisplayName = "Warded"),      // target warded against effect's damage source
};

USTRUCT()
struct KBS_API FTacLogPayload
{
	GENERATED_BODY()
};

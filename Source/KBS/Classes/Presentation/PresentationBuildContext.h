#pragma once
#include "CoreMinimal.h"

struct FAbilityUsePayload;
class UTacGridSubsystem;
class UTacticalPresentationBuilderConfig;
class UAbilityPresentationAsset;
class AUnit;

// Per-payload context assembled once by TacticalPresentationBuilder before the step loop.
// All converter functions take this instead of the individual parameters they previously carried.
struct FPresentationBuildContext
{
    // Null when the current event is not an ability-use payload.
    const FAbilityUsePayload* AbilityPayload = nullptr;

    // Unit lookup built from current grid state; valid for the entire Build call.
    const TMap<FGuid, AUnit*>* UnitLookup = nullptr;

    // Grid subsystem; valid for the entire Build call.
    UTacGridSubsystem* GridSubsystem = nullptr;

    // Null means every action falls back to its own hardcoded defaults.
    UTacticalPresentationBuilderConfig* Config = nullptr;

    // Null when the ability carries no presentation manifest.
    UAbilityPresentationAsset* Manifest = nullptr;

    // EventId of the current spine entry; used for reproducible seeding in the delivery phase.
    FGuid EventId;
};

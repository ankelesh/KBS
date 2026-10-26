#pragma once
#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "GameplayTypes/LogTypesLibrary.h"
#include "GameplayTypes/TeamConstants.h"
#include "GameplayTypes/CombatTypes.h"

// ── Short enum helpers (inline) ───────────────────────────────────────────────

inline FString ShortOrigin(ETacLogEventOrigin O)
{
	switch (O)
	{
	case ETacLogEventOrigin::Initiated:  return TEXT("Init");
	case ETacLogEventOrigin::Reaction:   return TEXT("React");
	case ETacLogEventOrigin::Triggered:  return TEXT("Trig");
	case ETacLogEventOrigin::Progressed: return TEXT("Prog");
	}
	return TEXT("?");
}

inline FString ShortType(ETacLogEventType T)
{
	switch (T)
	{
	case ETacLogEventType::Ability:          return TEXT("Ability");
	case ETacLogEventType::EffectActivation: return TEXT("EffOn");
	case ETacLogEventType::EffectEnd:        return TEXT("EffOff");
	case ETacLogEventType::TurnChange:       return TEXT("Turn");
	case ETacLogEventType::UnitExitField:    return TEXT("Exit");
	case ETacLogEventType::UnitSpawn:        return TEXT("Spawn");
	case ETacLogEventType::UnitDespawn:      return TEXT("Despawn");
	}
	return TEXT("?");
}

inline FString ShortDespawnReason(EUnitDespawnReason R)
{
	switch (R)
	{
	case EUnitDespawnReason::DurationExpired: return TEXT("Expired");
	case EUnitDespawnReason::SummonerDied:    return TEXT("SummonerDied");
	case EUnitDespawnReason::Replaced:        return TEXT("Replaced");
	}
	return TEXT("?");
}

inline FString ShortTeam(ETeamSide S)
{
	return (S == ETeamSide::Attacker) ? TEXT("Atk") : TEXT("Def");
}

inline FString ShortPolicy(EStatModifierRemovalPolicy P)
{
	switch (P)
	{
	case EStatModifierRemovalPolicy::Permanent:          return TEXT("perm");
	case EStatModifierRemovalPolicy::InstaRemove:        return TEXT("insta");
	case EStatModifierRemovalPolicy::DurationControlled: return TEXT("dur");
	case EStatModifierRemovalPolicy::OwnerControlled:    return TEXT("own");
	}
	return TEXT("?");
}

inline FString ShortRemoval(EEffectRemovalReason R)
{
	switch (R)
	{
	case EEffectRemovalReason::Expired:   return TEXT("Expired");
	case EEffectRemovalReason::Dispelled: return TEXT("Dispelled");
	case EEffectRemovalReason::Replaced:  return TEXT("Replaced");
	case EEffectRemovalReason::OwnerDied: return TEXT("OwnerDied");
	}
	return TEXT("?");
}

inline FString ShortOutcome(EHitOutcome O)
{
	switch (O)
	{
	case EHitOutcome::Hit:       return TEXT("HIT");
	case EHitOutcome::Miss:      return TEXT("MISS");
	case EHitOutcome::Immune:    return TEXT("IMMUNE");
	case EHitOutcome::Warded:    return TEXT("WARDED");
	case EHitOutcome::Cancelled: return TEXT("CANCEL");
	}
	return TEXT("?");
}

inline FString AssetShortName(const FPrimaryAssetId& Id)
{
	return Id.PrimaryAssetName.ToString();
}

// ── Name series for unit labels ───────────────────────────────────────────────

inline const TArray<FString>& GetNameSeries()
{
	static const TArray<FString> Names = {
		TEXT("alpha"),   TEXT("beta"),    TEXT("gamma"),   TEXT("delta"),
		TEXT("epsilon"), TEXT("zeta"),    TEXT("eta"),     TEXT("theta"),
		TEXT("iota"),    TEXT("kappa"),   TEXT("lambda"),  TEXT("mu"),
		TEXT("nu"),      TEXT("xi"),      TEXT("omicron"), TEXT("pi"),
		TEXT("rho"),     TEXT("sigma"),   TEXT("tau"),     TEXT("upsilon"),
		TEXT("phi"),     TEXT("chi"),     TEXT("psi"),     TEXT("omega")
	};
	return Names;
}

inline FString MakeUnitLabel(const FString& TypeName, int32 Index)
{
	const TArray<FString>& Names = GetNameSeries();
	FString Suffix = Index < Names.Num()
		? Names[Index]
		: FString::Printf(TEXT("unit%d"), Index);
	return TypeName + TEXT("-") + Suffix;
}

// ── Format context (carries per-session Abil# / Fx# registries) ──────────────

struct FTacLogFormatCtx
{
	const TMap<FGuid, FString>& UnitNames;
	TMap<FGuid, int32> AbilRegistry;
	TMap<FGuid, int32> FxRegistry;
	int32 AbilCounter = 1;
	int32 FxCounter   = 1;

	FString Unit(FGuid Id) const
	{
		const FString* Name = UnitNames.Find(Id);
		if (Name) return *Name;
		return Id.IsValid() ? TEXT("?unit") : TEXT("-");
	}

	FString Abil(FGuid InstanceId, const FPrimaryAssetId& AssetId)
	{
		if (!AbilRegistry.Contains(InstanceId))
			AbilRegistry.Add(InstanceId, AbilCounter++);
		return FString::Printf(TEXT("Abil#%d [%s]"), AbilRegistry[InstanceId], *AssetShortName(AssetId));
	}

	FString Fx(FGuid InstanceId, const FPrimaryAssetId& AssetId)
	{
		if (!FxRegistry.Contains(InstanceId))
			FxRegistry.Add(InstanceId, FxCounter++);
		return FString::Printf(TEXT("Fx#%d [%s]"), FxRegistry[InstanceId], *AssetShortName(AssetId));
	}

	FString FxFromRef(const FAppliedEffectRef& Ref)
	{
		return Fx(Ref.InstanceId, Ref.AssetId);
	}
};

// ── Legend & dispatch (implemented in TacLogPrettyPrint.cpp) ─────────────────

struct FTacLogStepBase;
struct FTacLogPayload;

FString BuildTacLogLegend();
FString DispatchFormatPayload(const TInstancedStruct<FTacLogPayload>& Payload, FTacLogFormatCtx& Ctx);

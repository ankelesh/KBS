#pragma once
#include "CoreMinimal.h"
#include <type_traits>
#include "DescriptorOverridePolicies.generated.h"

UENUM(BlueprintType)
enum class EDescriptorOverridePolicy : uint8
{
	// Inline data ignored; asset values used as-is
	AssetOnly        UMETA(DisplayName = "Asset Only"),
	// Sentinel-check scalars, copy if non-default; bools skipped
	Merge            UMETA(DisplayName = "Merge"),
	// Sentinel-check scalars, copy if non-default; bools always copied
	MergeWithBools   UMETA(DisplayName = "Merge With Bools"),
	// Merge arrays/sets only; scalar and bool fields ignored
	Append           UMETA(DisplayName = "Append"),
	// Inline data used as-is; asset ignored even if set
	InlineOnly       UMETA(DisplayName = "Inline Only"),
};

// GetBase() returns integral; supports InitFromBase — matches FUnitStatPositive
template<typename T>
concept CIntegralBasedStat = requires(const T& ct, T& mt)
{
	requires std::is_integral_v<std::remove_cvref_t<decltype(ct.GetBase())>>;
	mt.InitFromBase(ct.GetBase());
};

// GetBase() returns container with IsEmpty(); supports InitFromBase — matches FDamageSourceSetStat
template<typename T>
concept CSetBasedStat = requires(const T& ct, T& mt)
{
	requires std::is_convertible_v<decltype(ct.GetBase().IsEmpty()), bool>;
	mt.InitFromBase(ct.GetBase());
};

// --- Extractor functions ---

// Scalar / enum: sentinel = specific value; Append falls back to Merge for scalars
template<typename T>
T PickScalar(const T& Asset, const T& Inline, const T& Sentinel, EDescriptorOverridePolicy Policy)
{
	if (Policy == EDescriptorOverridePolicy::AssetOnly)  return Asset;
	if (Policy == EDescriptorOverridePolicy::InlineOnly) return Inline;
	return (Inline != Sentinel) ? Inline : Asset;
}

// FText: sentinel = IsEmpty()
inline FText PickText(const FText& Asset, const FText& Inline, EDescriptorOverridePolicy Policy)
{
	if (Policy == EDescriptorOverridePolicy::AssetOnly)  return Asset;
	if (Policy == EDescriptorOverridePolicy::InlineOnly) return Inline;
	return !Inline.IsEmpty() ? Inline : Asset;
}

// bool: no sentinel; copied only under MergeWithBools or InlineOnly
inline bool PickBool(bool Asset, bool Inline, EDescriptorOverridePolicy Policy)
{
	return (Policy == EDescriptorOverridePolicy::MergeWithBools
		 || Policy == EDescriptorOverridePolicy::InlineOnly) ? Inline : Asset;
}

// TArray / appendable container: sentinel = IsEmpty(); Append = concat
template<typename T>
T PickContainer(const T& Asset, const T& Inline, EDescriptorOverridePolicy Policy)
{
	if (Policy == EDescriptorOverridePolicy::AssetOnly)  return Asset;
	if (Policy == EDescriptorOverridePolicy::InlineOnly) return Inline;
	if (Policy == EDescriptorOverridePolicy::Append)
	{
		if (Inline.IsEmpty()) return Asset;
		T Result = Asset;
		Result.Append(Inline);
		return Result;
	}
	// Merge, MergeWithBools
	return !Inline.IsEmpty() ? Inline : Asset;
}

// CIntegralBasedStat (e.g. FUnitStatPositive): sentinel = -1; returns fresh stat via InitFromBase
template<CIntegralBasedStat T>
T PickIntegralStat(const T& Asset, const T& Inline, int32 Sentinel, EDescriptorOverridePolicy Policy)
{
	if (Policy == EDescriptorOverridePolicy::AssetOnly) return Asset;
	if (Policy == EDescriptorOverridePolicy::InlineOnly)
	{
		T Result;
		Result.InitFromBase(Inline.GetBase());
		return Result;
	}
	// Merge, MergeWithBools, Append — scalars use sentinel check
	if (Inline.GetBase() != Sentinel)
	{
		T Result;
		Result.InitFromBase(Inline.GetBase());
		return Result;
	}
	return Asset;
}

// CSetBasedStat (e.g. FDamageSourceSetStat): sentinel = empty base; Append = union of bases
template<CSetBasedStat T>
T PickSetStat(const T& Asset, const T& Inline, EDescriptorOverridePolicy Policy)
{
	if (Policy == EDescriptorOverridePolicy::AssetOnly) return Asset;
	if (Policy == EDescriptorOverridePolicy::InlineOnly)
	{
		T Result;
		Result.InitFromBase(Inline.GetBase());
		return Result;
	}
	if (Policy == EDescriptorOverridePolicy::Append && !Inline.GetBase().IsEmpty())
	{
		auto Combined = Asset.GetBase();
		Combined.Append(Inline.GetBase());
		T Result;
		Result.InitFromBase(Combined);
		return Result;
	}
	// Merge, MergeWithBools (and Append with empty inline)
	if (!Inline.GetBase().IsEmpty())
	{
		T Result;
		Result.InitFromBase(Inline.GetBase());
		return Result;
	}
	return Asset;
}

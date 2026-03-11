#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "TargetingDescriptor.generated.h"

// Hints HasAnyValidTargets which fast-path to take — one entry per distinct check category.
enum class ETargetFastCheck : uint8
{
	Full,        // no fast path; run full cell query
	Affiliation, // team alive+on-field check, branching on Affiliation
	EmptyCell,   // TestCells with empty predicate
	Corpse,      // TestCorpseCells
	Movement,    // TestCells with move predicate (or fall through for multi-cell ground)
};

// Primary dispatch axis — what kind of targeting algorithm runs
UENUM(BlueprintType)
enum class ETargetingStrategy : uint8
{
	None        UMETA(DisplayName = "None"),
	Self        UMETA(DisplayName = "Self"),
	Closest     UMETA(DisplayName = "Closest"),       // adjacent only
	Single      UMETA(DisplayName = "Single"),         // any one cell by affiliation
	All         UMETA(DisplayName = "All"),             // every cell by affiliation
	Area        UMETA(DisplayName = "Area"),            // shape-relative cells
	EmptyCell   UMETA(DisplayName = "Empty Cell"),
	Movement    UMETA(DisplayName = "Movement"),
	Corpse      UMETA(DisplayName = "Corpse"),
};

UENUM(BlueprintType)
enum class ETargetAffiliation : uint8
{
	Any      UMETA(DisplayName = "Any"),
	Enemy    UMETA(DisplayName = "Enemy"),
	Friendly UMETA(DisplayName = "Friendly"),
};

UENUM(BlueprintType)
enum class EMovementPattern : uint8
{
	Orthogonal UMETA(DisplayName = "Orthogonal"),
	AnyToAny   UMETA(DisplayName = "Any To Any"),
	Linear     UMETA(DisplayName = "Linear"),
};

UENUM(BlueprintType)
enum class EMovementLayer : uint8
{
	Ground     UMETA(DisplayName = "Ground"),
	Air        UMETA(DisplayName = "Air"),
	CrossLayer UMETA(DisplayName = "Cross Layer"),  // switch between layers
};

USTRUCT(BlueprintType)
struct FTargetingDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	ETargetingStrategy Strategy = ETargetingStrategy::None;

	// Relevant for: Single, All, Closest, Area, Corpse (Closest also requires adjacency/range check)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	ETargetAffiliation Affiliation = ETargetAffiliation::Enemy;

	// Relevant for: Movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	EMovementPattern MovementPattern = EMovementPattern::Orthogonal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	EMovementLayer MovementLayer = EMovementLayer::Ground;

	// Relevant for: Corpse — whether corpses under live units are valid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	bool bAllowCoveredCorpse = true;

	// Relevant for: Single — allows targeting empty cells alongside affiliation matches
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	bool bAllowEmpty = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	bool bAllowFlank = true;

	bool operator==(const FTargetingDescriptor&) const;

	// Converts editor-facing enum to runtime descriptor.
	// Single registration point — adding a new ETargetReach value costs one line in the .cpp table.
	static FTargetingDescriptor FromReach(ETargetReach Reach);

	bool IsFriendly() const { return Affiliation == ETargetAffiliation::Friendly; }
	bool IsMovement() const { return Strategy == ETargetingStrategy::Movement; }
	bool IsCorpse()   const { return Strategy == ETargetingStrategy::Corpse; }

	ETargetFastCheck GetFastCheckHint() const;
};

FORCEINLINE uint32 GetTypeHash(const FTargetingDescriptor& D)
{
	return HashCombine(GetTypeHash(static_cast<uint8>(D.Strategy)),
	       HashCombine(GetTypeHash(static_cast<uint8>(D.Affiliation)),
	       HashCombine(GetTypeHash(static_cast<uint8>(D.MovementPattern)),
	       HashCombine(GetTypeHash(static_cast<uint8>(D.MovementLayer)),
	       HashCombine(GetTypeHash(D.bAllowCoveredCorpse), GetTypeHash(D.bAllowEmpty), GetTypeHash(D.bAllowFlank))))));
}

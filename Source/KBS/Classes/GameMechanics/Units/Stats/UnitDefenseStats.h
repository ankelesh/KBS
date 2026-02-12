#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/CombatConstants.h"
#include "GameMechanics/Units/Stats/BaseUnitStatTypes.h"
#include "UnitDefenseStats.generated.h"

// Immunity modifiers
USTRUCT(BlueprintType)
struct FUnitImmunityModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid EffectId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageSource DamageSource = EDamageSource::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsGranting = true;

	FUnitImmunityModifier() = default;
	FUnitImmunityModifier(FGuid Id, EDamageSource Source, bool bGranting = true)
		: EffectId(Id), DamageSource(Source), bIsGranting(bGranting) {}

	bool operator==(const FUnitImmunityModifier& Other) const
	{
		return EffectId == Other.EffectId && DamageSource == Other.DamageSource && bIsGranting == Other.bIsGranting;
	}
};

USTRUCT(BlueprintType)
struct FUnitImmunities
{
	GENERATED_BODY()

	// === Public API ===
	void AddModifier(const FGuid& Id, EDamageSource Source, bool bIsGranting = true);
	void RemoveModifier(const FGuid& Id, EDamageSource Source, bool bIsGranting = true);
	bool IsImmuneTo(EDamageSource Source) const;
	const TSet<EDamageSource>& GetBase() const { return BaseImmunities; }
	void InitFromBase(const FUnitImmunities& Template);

	FUnitImmunities();
	explicit FUnitImmunities(TSet<EDamageSource> Immunities);

private:
	// === Internal State ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense", meta = (AllowPrivateAccess = "true"))
	TSet<EDamageSource> BaseImmunities;

	UPROPERTY()
	mutable TSet<EDamageSource> ModifiedImmunities;

	UPROPERTY()
	TArray<FUnitImmunityModifier> ModifyingEffects;

	UPROPERTY()
	mutable bool bIsDirty = false;

	// === Internal Methods ===
	void Recalc() const;
	TSet<EDamageSource> CalcAdditions() const;
	TSet<EDamageSource> CalcSubtractions() const;
};

USTRUCT(BlueprintType)
struct FUnitWards
{
	GENERATED_BODY()

	void Add(EDamageSource Source);
	void Remove(EDamageSource Source);
	bool HasWardFor(EDamageSource Source) const;
	bool UseWard(EDamageSource Source);
	void InitFromBase(const FUnitWards& Template);
	const TSet<EDamageSource>& GetWards() const { return Wards; }

	FUnitWards() = default;
	explicit FUnitWards(TSet<EDamageSource> Source);

	UPROPERTY()
	TSet<EDamageSource> Wards;
};

USTRUCT(BlueprintType)
struct FUnitArmourModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid EffectId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageSource Source = EDamageSource::None;

	FUnitArmourModifier() = default;
	FUnitArmourModifier(int32 InAmount, FGuid InEffectId, EDamageSource InSource)
		: Amount(InAmount), EffectId(InEffectId), Source(InSource) {}

	bool operator==(const FUnitArmourModifier& Other) const
	{
		return Amount == Other.Amount && EffectId == Other.EffectId && Source == Other.Source;
	}
};

USTRUCT(BlueprintType)
struct FUnitArmour
{
	GENERATED_BODY()

	// === Public API ===
	void AddFlatModifier(const FGuid& EffectId, int32 Amount, EDamageSource Source);
	void RemoveFlatModifier(const FGuid& EffectId, int32 Amount, EDamageSource Source);
	void AddMultiplier(const FGuid& EffectId, int32 Amount, EDamageSource Source);
	void RemoveMultiplier(const FGuid& EffectId, int32 Amount, EDamageSource Source);
	void AddOverride(const FGuid& EffectId, int32 Amount, EDamageSource Source);
	void RemoveOverride(const FGuid& EffectId, int32 Amount, EDamageSource Source);

	int32 GetValue(EDamageSource Src) const;
	void SetBase(int32 NewBase, EDamageSource Src);
	const TMap<EDamageSource, int32>& GetBaseArmour() const { return BaseArmour; }
	void InitFromBase(const FUnitArmour& Template);

	FUnitArmour();
	explicit FUnitArmour(TMap<EDamageSource, int32> Base);

private:
	// === Internal State ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense", meta = (AllowPrivateAccess = "true"))
	TMap<EDamageSource, int32> BaseArmour;

	UPROPERTY()
	mutable TMap<EDamageSource, int32> ModifiedArmour;

	UPROPERTY()
	TArray<FUnitArmourModifier> FlatModifiers;

	UPROPERTY()
	TArray<FUnitArmourModifier> MultiplierModifiers;

	UPROPERTY()
	TArray<FUnitArmourModifier> OverridingModifiers;

	UPROPERTY()
	mutable bool bIsDirty = false;

	// === Internal Methods ===
	bool IsDirty() const;
	void Recalc() const;
	void CalcFlatModifiers() const;
	void CalcArmourMultipliers() const;
	void CalcOverrides() const;
	void ClampArmour() const;

	static TMap<EDamageSource, int32> InitArmourMap();
};

// Defense stats aggregator
USTRUCT(BlueprintType)
struct FUnitDefenseStats
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	FUnitWards Wards;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	FUnitImmunities Immunities;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	FUnitArmour Armour;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	FUnitStatImmutableInt32 DamageReduction {0};


	void InitFromBase(const FUnitDefenseStats& Template);
};

#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/DamageTypes.h"
#include "BaseUnitStatTypes.generated.h"

// Stat modifier used by modifiable stats
USTRUCT(BlueprintType)
struct FInt32StatModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid EffectId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;

	FInt32StatModifier() = default;
	FInt32StatModifier(const FGuid InEffectId, const int32 InAmount) : EffectId(InEffectId), Amount(InAmount) {}

	bool operator==(const FInt32StatModifier& Other) const
	{
		return EffectId == Other.EffectId && Amount == Other.Amount;
	}
};

// Percentage stat (0-100)
USTRUCT(BlueprintType)
struct FUnitStatPercent
{
	GENERATED_BODY()

	// === Public API ===
	void AddFlatModifier(const FGuid& EffectId, int32 Amount);
	void RemoveFlatModifier(const FGuid& EffectId, int32 Amount);
	void AddMultiplier(const FGuid& EffectId, int32 Amount);
	void RemoveMultiplier(const FGuid& EffectId, int32 Amount);

	int32 GetValue();
	int32 GetBase() const { return Base; }
	void SetBase(int32 NewBase);

	FUnitStatPercent() : Base(0), Modified(0), bIsDirty(false) {}
	explicit FUnitStatPercent(int32 InBase);

private:
	// === Internal State ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	int32 Base = 0;

	UPROPERTY()
	int32 Modified = 0;

	UPROPERTY()
	TArray<FInt32StatModifier> FlatModifiers;

	UPROPERTY()
	TArray<FInt32StatModifier> StatMultipliers;

	UPROPERTY()
	bool bIsDirty = false;

	// === Internal Methods ===
	bool IsDirty() const;
	void Recalc();
	int32 CalcFlatModifiers();
	float CalcStatMultipliers();
};

// Positive stat (min=0, no max)
USTRUCT(BlueprintType)
struct FUnitStatPositive
{
	GENERATED_BODY()

	// === Public API ===
	void AddFlatModifier(const FGuid& EffectId, int32 Amount);
	void RemoveFlatModifier(const FGuid& EffectId, int32 Amount);
	void AddMultiplier(const FGuid& EffectId, int32 Amount);
	void RemoveMultiplier(const FGuid& EffectId, int32 Amount);

	int32 GetValue();
	int32 GetBase() const { return Base; }
	void SetBase(int32 NewBase);

	FUnitStatPositive() : Base(0), Modified(0), bIsDirty(false) {}
	explicit FUnitStatPositive(int32 InBase);

private:
	// === Internal State ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	int32 Base = 0;

	UPROPERTY()
	int32 Modified = 0;

	UPROPERTY()
	TArray<FInt32StatModifier> FlatModifiers;

	UPROPERTY()
	TArray<FInt32StatModifier> StatMultipliers;

	UPROPERTY()
	bool bIsDirty = false;

	// === Internal Methods ===
	bool IsDirty() const;
	void Recalc();
	int32 CalcFlatModifiers();
	float CalcStatMultipliers();
};

// Immutable stat - set once at initialization, never modified (prevents balance-breaking modifications)
USTRUCT(BlueprintType)
struct FUnitStatImmutableInt32
{
	GENERATED_BODY()

	int32 GetValue() const { return Value; }
	operator int32() const { return Value; }

	FUnitStatImmutableInt32() = default;
	explicit FUnitStatImmutableInt32(int32 InValue) : Value(InValue) {}

	// Enforce immutability - catch balance-breaking attempts at compile time
	FUnitStatImmutableInt32& operator=(const FUnitStatImmutableInt32&) = delete;
	FUnitStatImmutableInt32& operator=(int32) = delete;
	FUnitStatImmutableInt32& operator+=(int32) = delete;
	FUnitStatImmutableInt32& operator-=(int32) = delete;
	FUnitStatImmutableInt32(const FUnitStatImmutableInt32&) = default;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Value = 0;
};

// DamageSource set modifier
USTRUCT(BlueprintType)
struct FDamageSourceSetModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid EffectId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<EDamageSource> Sources;

	FDamageSourceSetModifier() = default;
	FDamageSourceSetModifier(const FGuid InEffectId, const TSet<EDamageSource>& InSources)
		: EffectId(InEffectId), Sources(InSources) {}

	bool operator==(const FDamageSourceSetModifier& Other) const
	{
		return EffectId == Other.EffectId;
	}
};

// DamageSource set stat - modifiers can only add sources, never remove from base
USTRUCT(BlueprintType)
struct FDamageSourceSetStat
{
	GENERATED_BODY()

	// === Public API ===
	void AddModifier(const FGuid& EffectId, const TSet<EDamageSource>& Sources);
	void RemoveModifier(const FGuid& EffectId);

	const TSet<EDamageSource>& GetValue();
	const TSet<EDamageSource>& GetBase() const { return Base; }
	void SetBase(const TSet<EDamageSource>& NewBase);

	FDamageSourceSetStat() : bIsDirty(false) {}
	explicit FDamageSourceSetStat(const TSet<EDamageSource>& InBase);

private:
	// === Internal State ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	TSet<EDamageSource> Base;

	UPROPERTY()
	TSet<EDamageSource> Modified;

	UPROPERTY()
	TArray<FDamageSourceSetModifier> Modifiers;

	UPROPERTY()
	bool bIsDirty = false;

	// === Internal Methods ===
	bool IsDirty() const;
	void Recalc();
};

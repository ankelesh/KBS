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

	int32 GetValue() const;
	int32 GetBase() const { return Base; }
	void SetBase(int32 NewBase);
	void InitFromBase(int32 InBase);

	FUnitStatPercent() : Base(0), Modified(0), bIsDirty(false) {}
	explicit FUnitStatPercent(int32 InBase);

private:
	// === Internal State ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	int32 Base = 0;

	UPROPERTY()
	mutable int32 Modified = 0;

	UPROPERTY()
	TArray<FInt32StatModifier> FlatModifiers;

	UPROPERTY()
	TArray<FInt32StatModifier> StatMultipliers;

	UPROPERTY()
	mutable bool bIsDirty = false;

	// === Internal Methods ===
	bool IsDirty() const;
	void Recalc() const;
	int32 CalcFlatModifiers() const;
	float CalcStatMultipliers() const;
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

	int32 GetValue() const;
	int32 GetBase() const { return Base; }
	void SetBase(int32 NewBase);
	void InitFromBase(int32 InBase);

	FUnitStatPositive() : Base(0), Modified(0), bIsDirty(false) {}
	explicit FUnitStatPositive(int32 InBase);

private:
	// === Internal State ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	int32 Base = 0;

	UPROPERTY()
	mutable int32 Modified = 0;

	UPROPERTY()
	TArray<FInt32StatModifier> FlatModifiers;

	UPROPERTY()
	TArray<FInt32StatModifier> StatMultipliers;

	UPROPERTY()
	mutable bool bIsDirty = false;

	// === Internal Methods ===
	bool IsDirty() const;
	void Recalc() const;
	int32 CalcFlatModifiers() const;
	float CalcStatMultipliers() const;
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
	FUnitStatImmutableInt32(const FUnitStatImmutableInt32&) = default;

	// Runtime immutability enforcement - catches modifications while allowing UE's code generation
	FUnitStatImmutableInt32& operator=(const FUnitStatImmutableInt32& Other)
	{
		if (Value != Other.Value)
		{
			UE_LOG(LogTemp, Error, TEXT("Immutable stat modification attempt: %d -> %d"), Value, Other.Value);
			checkf(false, TEXT("Attempted to modify immutable stat after initialization"));
		}
		Value = Other.Value;
		return *this;
	}

	FUnitStatImmutableInt32& operator=(int32 NewValue)
	{
		if (Value != NewValue)
		{
			UE_LOG(LogTemp, Error, TEXT("Immutable stat modification attempt: %d -> %d"), Value, NewValue);
			checkf(false, TEXT("Attempted to modify immutable stat after initialization"));
		}
		Value = NewValue;
		return *this;
	}

	FUnitStatImmutableInt32& operator+=(int32 Delta)
	{
		UE_LOG(LogTemp, Error, TEXT("Immutable stat += operation attempted on value %d"), Value);
		checkf(false, TEXT("Attempted to modify immutable stat with += operator"));
		return *this;
	}

	FUnitStatImmutableInt32& operator-=(int32 Delta)
	{
		UE_LOG(LogTemp, Error, TEXT("Immutable stat -= operation attempted on value %d"), Value);
		checkf(false, TEXT("Attempted to modify immutable stat with -= operator"));
		return *this;
	}

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

	const TSet<EDamageSource>& GetValue() const;
	const TSet<EDamageSource>& GetBase() const { return Base; }
	void SetBase(const TSet<EDamageSource>& NewBase);
	void InitFromBase(const TSet<EDamageSource>& InBase);

	FDamageSourceSetStat() : bIsDirty(false) {}
	explicit FDamageSourceSetStat(const TSet<EDamageSource>& InBase);

private:
	// === Internal State ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	TSet<EDamageSource> Base;

	UPROPERTY()
	mutable TSet<EDamageSource> Modified;

	UPROPERTY()
	TArray<FDamageSourceSetModifier> Modifiers;

	UPROPERTY()
	mutable bool bIsDirty = false;

	// === Internal Methods ===
	bool IsDirty() const;
	void Recalc() const;
};

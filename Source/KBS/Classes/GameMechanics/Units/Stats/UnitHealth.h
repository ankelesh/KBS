#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Stats/BaseUnitStatTypes.h"
#include "UnitHealth.generated.h"

// Health wrapper - encapsulates current/max relationship
USTRUCT(BlueprintType)
struct FUnitHealth
{
	GENERATED_BODY()

	// === Public API ===
	// Max HP modification
	void SetMaxBase(int32 NewBase, bool bShouldHeal = true);
	void AddMaxModifier(const FGuid& EffectId, int32 Amount, bool bShouldHeal = true);
	void AddMaxMultiplier(const FGuid& EffectId, int32 Amount, bool bShouldHeal = true);
	void RemoveMaxModifier(const FGuid& EffectId, int32 Amount);
	void RemoveMaxMultiplier(const FGuid& EffectId, int32 Amount);

	// Current HP modification
	void TakeDamage(int32 Amount);
	void Heal(int32 Amount);
	void SetCurrent(int32 NewCurrent);
	void FullHeal();

	// Queries
	int32 GetMaximum();
	int32 GetCurrent() const { return Current; }
	bool IsDead() const { return Current <= 0; }
	float GetHealthPercent();

	FUnitHealth() : Maximum(100), Current(100) {}
	explicit FUnitHealth(int32 MaxHP) : Maximum(MaxHP), Current(MaxHP) {}

private:
	// === Internal State ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	FUnitStatPositive Maximum;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	int32 Current = 0;

	// === Internal Methods ===
	void ClampCurrent();
};

#pragma once
#include "CoreMinimal.h"
#include "UnitStatusContainer.generated.h"

UENUM(BlueprintType)
enum class EUnitStatus : uint8
{
	TurnBlocked,
	Pinned,
	Silenced,
	Disoriented,
	Focused,
	Fleeing,
	Channeling,
	Defending,
	Dead,
};

USTRUCT(BlueprintType)
struct FUnitStatusContainer
{
	GENERATED_BODY()

	// === Public API ===

	// Returns true if status was newly activated (went from 0 modifiers to 1, or bool flipped)
	bool AddStatus(EUnitStatus Status, const FGuid& EffectId);
	void SetFocus() {bFocused = true;}
	void SetFleeing() {bFleeing = true;}
	void SetChanneling() {bChanneling = true;}
	void SetDefending() {bDefending = true;}
	void SetDead() {bDead = true;}
	void SetFlankDelay(int32 Turns) { FlankDelay = FMath::Max(0, Turns); }
	void TickFlankDelay()           { if (FlankDelay > 0) --FlankDelay; }
	void BlockTurn(FGuid const& EffectId) { TurnBlockedModifiers.Add(EffectId); }
	void Pin(FGuid const& EffectId) { PinnedModifiers.Add(EffectId); }
	void Silence(FGuid const& EffectId) { SilencedModifiers.Add(EffectId); }
	void Disorient(FGuid const& EffectId) { DisorientedModifiers.Add(EffectId); }

	// Returns true if status was fully deactivated
	bool RemoveStatus(EUnitStatus Status, const FGuid& EffectId);

	void ClearStatus(EUnitStatus Status);
	void ClearAll();

	// Query API
	bool CanAct() const;
	bool CanMove() const;
	bool CanUseSpellbook() const;
	bool CanUseNonBasicAbilities() const;

	bool IsFocused() const { return bFocused; }
	bool IsFleeing() const { return bFleeing; }
	bool IsChanneling() const { return bChanneling; }
	bool IsDefending() const { return bDefending; }
	bool IsDead() const { return bDead; }
	bool IsFlankDelayed() const  { return FlankDelay > 0; }
	int32 FlankTurnsLeft() const { return FlankDelay; }
	bool HasReplacementBehavior() const { return bFleeing || bChanneling; }

	bool IsStatusActive(EUnitStatus Status) const;

private:
	// === Internal State ===

	// Ref-counted statuses
	UPROPERTY()
	TArray<FGuid> TurnBlockedModifiers;

	UPROPERTY()
	TArray<FGuid> PinnedModifiers;

	UPROPERTY()
	TArray<FGuid> SilencedModifiers;

	UPROPERTY()
	TArray<FGuid> DisorientedModifiers;

	// Counter statuses
	UPROPERTY()
	int32 FlankDelay = 0;

	// Bool statuses
	UPROPERTY()
	bool bFocused = false;

	UPROPERTY()
	bool bFleeing = false;

	UPROPERTY()
	bool bChanneling = false;
	
	UPROPERTY()
	bool bDefending = false;

	UPROPERTY()
	bool bDead = false;
};

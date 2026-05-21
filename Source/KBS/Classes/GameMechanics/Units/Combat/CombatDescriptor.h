#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/TargetingDescriptor.h"
#include "GameMechanics/Units/Combat/CombatDescriptorData.h"
#include "CombatDescriptor.generated.h"
class UBattleEffect;
class AUnit;
UCLASS(BlueprintType)
class KBS_API UCombatDescriptor : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(UObject* Outer, const FCombatDescriptorData& Data);

	
	// Add enchantments/buffs to descriptor stats (e.g., Stats.BaseMagnitude.AddFlatModifier(...))
	void ModifyMagnitude(int32 Magnitude, FGuid ModificatorGuid, bool bIsFlat);
	void RemoveMagnitudeModifier(int32 Magnitude, FGuid ModificatorGuid, bool bIsFlat);
	void ModifySource(const TSet<EDamageSource>& Sources, FGuid ModificatorGuid);
	void RemoveSourceModifier(FGuid ModificatorGuid);

	bool IsMutable() const;
	void SetMagnitudeBase(int32 Magnitude);
	bool IsRequiringAccuracyRoll() const { return bGuaranteedHit;};
	EMagnitudePolicy GetMagnitudePolicy() const { return MagnitudePolicy; }
	const FCombatDescriptorStats& GetStats() const { return Stats; }
	const TArray<UBattleEffect*>& GetEffects() const { return ActiveEffects; }
	FText GetEffectsTooltips(AUnit* Owner);
	FTargetingDescriptor GetTargeting() const { return FTargetingDescriptor::FromReach(Stats.TargetReach); }
	const FDescriptorSideEffects& GetSideEffects() const { return SideEffects; }
protected:
	// Single stats instance - stat wrappers handle Base/Modified internally
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	EMagnitudePolicy MagnitudePolicy = EMagnitudePolicy::Damage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Descriptor")
	FCombatDescriptorStats Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	bool bIsImmutable;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	bool bGuaranteedHit;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Side Effects")
	FDescriptorSideEffects SideEffects;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Descriptor")
	TArray<TObjectPtr<UBattleEffect>> ActiveEffects;
};


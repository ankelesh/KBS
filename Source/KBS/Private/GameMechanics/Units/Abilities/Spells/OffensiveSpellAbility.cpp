#include "GameMechanics/Units/Abilities/Spells/OffensiveSpellAbility.h"
#include "GameMechanics/Units/Abilities/Spells/OffensiveSpellAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/PresentationSubsystem.h"
#include "GameplayTypes/CombatTypes.h"


void UOffensiveSpellAbility::InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner)
{
	Super::InitializeFromDefinition(InDefinition, InOwner);

	UOffensiveSpellAbilityDefinition* SpellConfig = Cast<UOffensiveSpellAbilityDefinition>(Config);
	checkf(SpellConfig, TEXT("UOffensiveSpellAbility requires a USpellAbilityDefinition asset"));

	UWeaponDataAsset* TransientWeapon = NewObject<UWeaponDataAsset>(this);
	TransientWeapon->BaseStats = SpellConfig->EmbeddedStats;
	TransientWeapon->Effects = SpellConfig->EmbeddedEffects;
	TransientWeapon->AttackMontage = SpellConfig->AttackMontage;
	TransientWeapon->Designation = EWeaponDesignation::Spells;

	EmbeddedWeapon = NewObject<UWeapon>(this);
	EmbeddedWeapon->Initialize(this, TransientWeapon);
}

void UOffensiveSpellAbility::ScaleEmbeddedWeapon() const
{
	UOffensiveSpellAbilityDefinition* SpellConfig = Cast<UOffensiveSpellAbilityDefinition>(Config);
	if (SpellConfig->bIgnoreScaling) return;

	UWeapon* SpellWeapon = FDamageCalculation::SelectSpellWeapon(Owner);
	checkf(SpellWeapon, TEXT("UOffensiveSpellAbility: unit has no Spells-designated weapon to drive scaling"));

	FDamageCalculation::ApplySpellScaling(EmbeddedWeapon, SpellWeapon, SpellConfig->DamageMultiplier, SpellConfig->FlatDamageBonus);
}

TMap<FTacCoordinates, FPreviewHitResult> UOffensiveSpellAbility::DamagePreview(FTacCoordinates TargetCell) const
{
	TMap<FTacCoordinates, FPreviewHitResult> Results;
	if (!Owner) return Results;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return Results;

	ScaleEmbeddedWeapon();

	FResolvedTargets ResolvedTargets = TargetingService->ResolveTargetsFromClick(Owner, TargetCell, GetTargeting(), &EmbeddedWeapon->GetStats().AreaShape);
	for (AUnit* Target : ResolvedTargets.GetAllTargets())
	{
		if (!Target) continue;
		FPreviewHitResult Preview = FDamageCalculation::PreviewDamage(Owner, EmbeddedWeapon, Target);
		Results.Add(Target->GetGridMetadata().Coords, Preview);
	}
	return Results;
}

bool UOffensiveSpellAbility::Execute(FTacCoordinates TargetCell)
{
	if (!Owner) return false;

	UTacCombatSubsystem* CombatSubsystem = GetCombatSubsystem();
	if (!CombatSubsystem) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	FResolvedTargets ResolvedTargets = TargetingService->ResolveTargetsFromClick(Owner, TargetCell, GetTargeting(), &EmbeddedWeapon->GetStats().AreaShape);
	if (!ResolvedTargets.ClickedTarget) return false;

	ScaleEmbeddedWeapon();

	UPresentationSubsystem::FScopedBatch SpellBatch(
		UPresentationSubsystem::Get(Owner),
		FString::Printf(TEXT("Spell_%s"), *Owner->GetName())
	);

	if (Owner->VisualsComponent)
	{
		Owner->VisualsComponent->PlayAttackSequence(Owner, ResolvedTargets.ClickedTarget, EmbeddedWeapon);
	}

	TArray<AUnit*> AllTargets = ResolvedTargets.GetAllTargets();
	TArray<FCombatHitResult> HitResults = CombatSubsystem->ResolveAttack(Owner, AllTargets, EmbeddedWeapon);

	bool bProcessingSucceeded = true;
	for (const FCombatHitResult& HitResult : HitResults)
	{
		if (!HitResult.bProcessingSucceeded)
		{
			bProcessingSucceeded = false;
		}
	}

	Owner->GetStats().Status.SetFocus();
	ConsumeCharge();

	return bProcessingSucceeded;
}

bool UOffensiveSpellAbility::CanExecute(FTacCoordinates TargetCell) const
{
	if (!Owner || RemainingCharges <= 0 || IsOutsideFocus()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	return TargetingService->HasValidTargetAtCell(Owner, TargetCell, GetTargeting());
}

bool UOffensiveSpellAbility::CanExecute() const
{
	if (!Owner || RemainingCharges <= 0 || IsOutsideFocus()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	return TargetingService->HasAnyValidTargets(Owner, GetTargeting());
}

ETargetReach UOffensiveSpellAbility::GetTargeting() const
{
	if (Config->Targeting != ETargetReach::None)
	{
		return Config->Targeting;
	}
	return EmbeddedWeapon->GetReach();
}

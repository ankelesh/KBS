#include "Presentation/Tactical/Converters/CombatStepConverter.h"
#include "Presentation/KBSPresentationLog.h"
#include "Presentation/Assets/AbilityPresentationAsset.h"
#include "Presentation/Tactical/TacticalPresentationBuilderConfig.h"
#include "Presentation/Tactical/Actions/DeliveryPresentationAction.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Components/UnitVisualsComponent.h"
#include "GameplayTypes/Tags/Visual/UnitVisualTags.h"
#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"
#include "Presentation/Core/Actions/MontagePresentationAction.h"
#include "Presentation/Tactical/Actions/AoEReactionPresentationAction.h"
#include "Math/RandomStream.h"

namespace
{
	bool ResolveMontageContext(AUnit* Actor, FGameplayTag Tag, FName EarlyExitNotifyName, FMontageActionContext& OutContext)
	{
		if (!Tag.IsValid())
		{
			return false;
		}

		UAnimMontage* Montage = Actor->GetVisualsComponent()->ResolveAnimation(Tag);
		if (!Montage)
		{
			return false;
		}

		OutContext.Actor = Actor;
		OutContext.Montage = Montage;
		OutContext.EarlyExitNotifyName = EarlyExitNotifyName;
		return true;
	}

	UMontagePresentationAction* MakeMontageAction(AUnit* Actor, FGameplayTag Tag, FName EarlyExitNotifyName = NAME_None)
	{
		FMontageActionContext Context;
		if (!ResolveMontageContext(Actor, Tag, EarlyExitNotifyName, Context))
		{
			if (CVarDumpSequence.GetValueOnGameThread())
			{
				UE_LOG(LogKBSPresentation, Warning,
					TEXT("[PresentationDump] MakeMontageAction: no montage for tag '%s' on unit '%s' - slot skipped"),
					*Tag.ToString(), *Actor->GetName());
			}
			return nullptr;
		}

		UMontagePresentationAction* Action = NewObject<UMontagePresentationAction>();
		Action->Context = Context;
		return Action;
	}

	UFloatingTextPresentationAction* MakeHitText(AUnit* Target, const FTacLogHitRecord& Hit,
	                                             const UTacticalPresentationBuilderConfig* Config)
	{
		UFloatingTextPresentationAction* Action = NewObject<UFloatingTextPresentationAction>();
		Action->Context.Actor = Target;

		switch (Hit.Outcome)
		{
		case EHitOutcome::Hit:
			Action->Context.Text = FText::AsNumber(Hit.DamageResult.Damage);
			Action->Context.Color = Config ? Config->DamageColor : FLinearColor::Red;
			return Action;
		case EHitOutcome::Miss:
			Action->Context.Text = NSLOCTEXT("CombatStepConverter", "Miss", "Miss");
			Action->Context.Color = Config ? Config->MissColor : FLinearColor::Gray;
			return Action;
		case EHitOutcome::Immune:
			Action->Context.Text = NSLOCTEXT("CombatStepConverter", "Immune", "Immune");
			Action->Context.Color = Config ? Config->ImmuneColor : FLinearColor::Blue;
			return Action;
		case EHitOutcome::Warded:
			Action->Context.Text = NSLOCTEXT("CombatStepConverter", "Warded", "Warded");
			Action->Context.Color = Config ? Config->WardedColor : FLinearColor(0.f, 0.8f, 0.8f);
			return Action;
		default:
			// Cancelled - nothing happened, nothing to show.
			return nullptr;
		}
	}

	// Builds the delivery action for this combat step from the manifest's Combat block.
	// Returns nullptr when topology is None or manifest is absent.
	UDeliveryPresentationAction* MakeDeliveryAction(const FTacLogCombatStep& Step,
	                                                const FPresentationBuildContext& Context)
	{
		if (!Context.Manifest || Context.Manifest->Combat.IsEmpty())
		{
			return nullptr;
		}

		const FDeliveryPresentation& Delivery = Context.Manifest->Combat;

		AUnit* const* FoundAttacker = Context.UnitLookup->Find(Step.AttackerId);
		if (!FoundAttacker)
		{
			// No attacker actor on field (e.g. DoT) - delivery requires a source position.
			return nullptr;
		}
		AActor* WorldContextActor = *FoundAttacker;
		const FVector AttackerPos = Context.GridSubsystem->GetCellWorldLocation(Step.AttackerCoords);

		UNiagaraSystem* TrailVFX  = Delivery.TrailVFX.Get();
		UNiagaraSystem* ImpactVFX = Delivery.ImpactVFX.Get();
		USoundBase*     TrailSFX  = Delivery.TrailSFX.Get();
		USoundBase*     ImpactSFX = Delivery.ImpactSFX.Get();

		UDeliveryPresentationAction* Action = NewObject<UDeliveryPresentationAction>();

		switch (Delivery.Topology)
		{
		case EDeliveryTopology::OnePerTarget:
		{
			for (int32 HitIdx = 0; HitIdx < Step.HitRecords.Num(); ++HitIdx)
			{
				const FTacLogHitRecord& Hit = Step.HitRecords[HitIdx];
				const FVector TargetPos = Context.GridSubsystem->GetCellWorldLocation(Hit.TargetCoords);

				FProjectileActionContext Proj;
				Proj.WorldContextActor = WorldContextActor;
				Proj.StartLocation     = AttackerPos;
				Proj.EndLocation       = TargetPos;
				Proj.Speed             = Delivery.Speed;
				Proj.TrailVFX          = TrailVFX;
				Proj.TrailSFX          = TrailSFX;
				Proj.ImpactVFX         = ImpactVFX;
				Proj.ImpactSFX         = ImpactSFX;
				Proj.ActorClass        = Delivery.ActorClass;
				Action->ProjectileContexts.Add(Proj);

				// Reproducible per-chain delay seeded from EventId + target index.
				if (Delivery.MaxStaggerDelay > 0.f)
				{
					FRandomStream Stream(HashCombine(GetTypeHash(Context.EventId), (uint32)HitIdx));
					Action->ChainStartDelays.Add(Stream.FRandRange(0.f, Delivery.MaxStaggerDelay));
				}
				else
				{
					Action->ChainStartDelays.Add(0.f);
				}
			}
			break;
		}

		case EDeliveryTopology::SingleToPoint:
		{
			checkf(Context.AbilityPayload,
				TEXT("SingleToPoint delivery requires an ability-use payload for the Command coordinate"));
			const FVector TargetPos = Context.GridSubsystem->GetCellWorldLocation(Context.AbilityPayload->Command);

			FProjectileActionContext Proj;
			Proj.WorldContextActor = WorldContextActor;
			Proj.StartLocation     = AttackerPos;
			Proj.EndLocation       = TargetPos;
			Proj.Speed             = Delivery.Speed;
			Proj.TrailVFX          = TrailVFX;
			Proj.TrailSFX          = TrailSFX;
			Proj.ImpactVFX         = ImpactVFX;
			Proj.ImpactSFX         = ImpactSFX;
			Proj.ActorClass        = Delivery.ActorClass;
			Action->ProjectileContexts.Add(Proj);
			Action->ChainStartDelays.Add(0.f);
			break;
		}

		case EDeliveryTopology::PerTargetVfx:
		{
			for (int32 HitIdx = 0; HitIdx < Step.HitRecords.Num(); ++HitIdx)
			{
				const FTacLogHitRecord& Hit = Step.HitRecords[HitIdx];
				FDeliveryVfxChain VfxChain;
				VfxChain.Location         = Context.GridSubsystem->GetCellWorldLocation(Hit.TargetCoords);
				VfxChain.ImpactVFX        = ImpactVFX;
				VfxChain.WorldContextActor = WorldContextActor;
				Action->VfxChains.Add(VfxChain);

				if (Delivery.MaxStaggerDelay > 0.f)
				{
					FRandomStream Stream(HashCombine(GetTypeHash(Context.EventId), (uint32)HitIdx));
					Action->ChainStartDelays.Add(Stream.FRandRange(0.f, Delivery.MaxStaggerDelay));
				}
				else
				{
					Action->ChainStartDelays.Add(0.f);
				}
			}
			break;
		}

		default:
			return nullptr;
		}

		const bool bHasChains = !Action->ProjectileContexts.IsEmpty() || !Action->VfxChains.IsEmpty();
		return bHasChains ? Action : nullptr;
	}
}

void TacticalLogConverters::ConvertCombatStep(const FTacLogCombatStep& Step,
                                              const FPresentationBuildContext& Context,
                                              TArray<TObjectPtr<UPresentationSequenceAction>>& OutActions)
{
	const bool bDump = CVarDumpSequence.GetValueOnGameThread() != 0;
	const int32 BaseActionCount = OutActions.Num();

	const FName ImpactNotify = Context.Config ? Context.Config->ImpactNotifyName : FName("Impact");
	const float HitReactionTimeout = Context.Config ? Context.Config->HitReactionTimeout : 1.5f;
	const float DeathTimeout       = Context.Config ? Context.Config->DeathTimeout       : 4.f;

	// Phase 1: Attacker swing montage.
	if (AUnit* const* FoundAttacker = Context.UnitLookup->Find(Step.AttackerId))
	{
		if (UMontagePresentationAction* SwingAction = MakeMontageAction(*FoundAttacker, Step.WeaponAnimTag, ImpactNotify))
		{
			OutActions.Add(SwingAction);
		}
	}

	// Phase 2: Delivery (projectile or per-target VFX).
	if (UDeliveryPresentationAction* DeliveryAction = MakeDeliveryAction(Step, Context))
	{
		OutActions.Add(DeliveryAction);
	}

	// Phase 3: Per-hit floating text labels.
	TArray<FAoEReactionChain> Chains;

	for (const FTacLogHitRecord& Hit : Step.HitRecords)
	{
		AUnit* const* FoundTarget = Context.UnitLookup->Find(Hit.TargetId);
		checkf(FoundTarget, TEXT("Combat step references unit %s - unit is present in UnitLookup"), *Hit.TargetId.ToString());
		AUnit* Target = *FoundTarget;

		if (UFloatingTextPresentationAction* HitText = MakeHitText(Target, Hit, Context.Config))
		{
			OutActions.Add(HitText);
		}

		if (Hit.Outcome != EHitOutcome::Hit)
		{
			continue;
		}

		// Phase 4: Reaction chain entry for this target.
		FAoEReactionChain Chain;

		if (FMontageActionContext ReactionContext; ResolveMontageContext(Target, TAG_ANIM_HIT_REACTION, NAME_None, ReactionContext))
		{
			Chain.Steps.Add(FAoEReactionStep{ReactionContext, HitReactionTimeout});
		}

		if (Hit.bKilledTarget)
		{
			if (FMontageActionContext DeathContext; ResolveMontageContext(Target, TAG_ANIM_DEATH, NAME_None, DeathContext))
			{
				// bMarksActorDead pushes bIsDead to the AnimBP once the montage finishes, so the
				// state machine holds the dead pose. Timeout is a safety net for errored assets.
				Chain.Steps.Add(FAoEReactionStep{DeathContext, DeathTimeout, /*bMarksActorDead=*/true});
			}
		}

		if (!Chain.Steps.IsEmpty())
		{
			Chains.Add(MoveTemp(Chain));
		}
	}

	// Phase 4: AoE reaction chains (all hit targets in parallel).
	if (!Chains.IsEmpty())
	{
		UAoEReactionPresentationAction* ReactionAction = NewObject<UAoEReactionPresentationAction>();
		ReactionAction->Chains = MoveTemp(Chains);
		OutActions.Add(ReactionAction);
	}

	// Phase 5: Attacker flourish - one montage action when at least one hit killed its target.
	const bool bAnyKilled = !Step.HitRecords.IsEmpty() &&
		Step.HitRecords.ContainsByPredicate([](const FTacLogHitRecord& R) { return R.bKilledTarget; });
	if (bAnyKilled)
	{
		if (AUnit* const* FoundAttacker = Context.UnitLookup->Find(Step.AttackerId))
		{
			if (UMontagePresentationAction* FlourishAction = MakeMontageAction(*FoundAttacker, TAG_ANIM_FLOURISH))
			{
				OutActions.Add(FlourishAction);
			}
		}
	}

	if (bDump)
	{
		const int32 Added   = OutActions.Num() - BaseActionCount;
		const FString EventStr = Context.EventId.IsValid() ? Context.EventId.ToString() : TEXT("(no event)");
		UE_LOG(LogKBSPresentation, Log,
			TEXT("[PresentationDump] CombatStep event=%s attacker=%s: +%d actions"),
			*EventStr, *Step.AttackerId.ToString(), Added);
	}
}

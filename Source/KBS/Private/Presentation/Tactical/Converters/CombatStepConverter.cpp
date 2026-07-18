#include "Presentation/Tactical/Converters/CombatStepConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Components/UnitVisualsComponent.h"
#include "GameplayTypes/Tags/Visual/UnitVisualTags.h"
#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"
#include "Presentation/Core/Actions/MontagePresentationAction.h"
#include "Presentation/Tactical/Actions/AoEReactionPresentationAction.h"

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
			return nullptr;
		}

		UMontagePresentationAction* Action = NewObject<UMontagePresentationAction>();
		Action->Context = Context;
		return Action;
	}

	UFloatingTextPresentationAction* MakeHitText(AUnit* Target, const FTacLogHitRecord& Hit)
	{
		UFloatingTextPresentationAction* Action = NewObject<UFloatingTextPresentationAction>();
		Action->Context.Actor = Target;

		switch (Hit.Outcome)
		{
		case EHitOutcome::Hit:
			Action->Context.Text = FText::AsNumber(Hit.DamageResult.Damage);
			Action->Context.Color = FLinearColor::Red;
			return Action;
		case EHitOutcome::Miss:
			Action->Context.Text = NSLOCTEXT("CombatStepConverter", "Miss", "Miss");
			Action->Context.Color = FLinearColor::Gray;
			return Action;
		case EHitOutcome::Immune:
			Action->Context.Text = NSLOCTEXT("CombatStepConverter", "Immune", "Immune");
			Action->Context.Color = FLinearColor::Blue;
			return Action;
		case EHitOutcome::Warded:
			Action->Context.Text = NSLOCTEXT("CombatStepConverter", "Warded", "Warded");
			Action->Context.Color = FLinearColor(0.f, 0.8f, 0.8f);
			return Action;
		default:
			// Cancelled - nothing happened, nothing to show.
			return nullptr;
		}
	}
}

void TacticalLogConverters::ConvertCombatStep(const FTacLogCombatStep& Step, const TMap<FGuid, AUnit*>& UnitLookup, TArray<TObjectPtr<UPresentationSequenceAction>>& OutActions)
{
	if (AUnit* const* FoundAttacker = UnitLookup.Find(Step.AttackerId))
	{
		if (UMontagePresentationAction* SwingAction = MakeMontageAction(*FoundAttacker, Step.WeaponAnimTag, FName("Impact")))
		{
			OutActions.Add(SwingAction);
		}
	}

	TArray<FAoEReactionChain> Chains;

	for (const FTacLogHitRecord& Hit : Step.HitRecords)
	{
		AUnit* const* FoundTarget = UnitLookup.Find(Hit.TargetId);
		checkf(FoundTarget, TEXT("Combat step references unit %s not found on grid"), *Hit.TargetId.ToString());
		AUnit* Target = *FoundTarget;

		if (UFloatingTextPresentationAction* HitText = MakeHitText(Target, Hit))
		{
			OutActions.Add(HitText);
		}

		if (Hit.Outcome != EHitOutcome::Hit)
		{
			continue;
		}

		FAoEReactionChain Chain;

		if (FMontageActionContext ReactionContext; ResolveMontageContext(Target, TAG_ANIM_HIT_REACTION, NAME_None, ReactionContext))
		{
			Chain.Steps.Add(FAoEReactionStep{ReactionContext, 1.5f});
		}

		if (Hit.bKilledTarget)
		{
			if (FMontageActionContext DeathContext; ResolveMontageContext(Target, TAG_ANIM_DEATH, NAME_None, DeathContext))
			{
				// bMarksActorDead pushes bIsDead to the AnimBP once the montage finishes, so the
				// state machine holds the dead pose. Timeout is just a safety net for errored assets.
				Chain.Steps.Add(FAoEReactionStep{DeathContext, 4.f, /*bMarksActorDead=*/true});
			}
		}

		if (!Chain.Steps.IsEmpty())
		{
			Chains.Add(MoveTemp(Chain));
		}
	}

	if (!Chains.IsEmpty())
	{
		UAoEReactionPresentationAction* ReactionAction = NewObject<UAoEReactionPresentationAction>();
		ReactionAction->Chains = MoveTemp(Chains);
		OutActions.Add(ReactionAction);
	}
}

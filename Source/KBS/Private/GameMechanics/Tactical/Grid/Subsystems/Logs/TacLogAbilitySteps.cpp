#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Units/Unit.h"

FTacLogCombatStep FTacLogCombatStep::Make(FGuid AttackerId, FTacCoordinates AttackerCoords, FGuid PrimaryTargetId, const TArray<FCombatHitResult>& HitResults)
{
	FTacLogCombatStep Step;
	Step.AttackerId = AttackerId;
	Step.AttackerCoords = AttackerCoords;
	Step.PrimaryTargetId = PrimaryTargetId;
	for (const FCombatHitResult& Hit : HitResults)
	{
		FTacLogHitRecord& Rec = Step.HitRecords.AddDefaulted_GetRef();
		Rec.TargetId = Hit.TargetUnit->GetUnitID();
		Rec.TargetCoords = Hit.TargetUnit->GetGridMetadata().Coords;
		Rec.Outcome = Hit.HitOutcome;
		Rec.DamageResult = Hit.DamageResult;
		Rec.AppliedEffects = Hit.AppliedEffects;
		Rec.RemainingHp = Hit.TargetUnit->GetStats().Health.GetCurrent();
		Rec.bKilledTarget = Hit.TargetUnit->IsDead();
	}
	return Step;
}

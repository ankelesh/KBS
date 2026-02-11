#include "GameMechanics/Units/Stats/UnitStats.h"

void FUnitCoreStats::InitFromBase(const FUnitCoreStats& Template)
{
	Health.InitFromBase(Template.Health);
	Initiative.InitFromBase(Template.Initiative.GetBase());
	Accuracy.InitFromBase(Template.Accuracy.GetBase());
	Defense.InitFromBase(Template.Defense);
	// Status starts clean (no modifiers)
	Status.ClearAll();
}

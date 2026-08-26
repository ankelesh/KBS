#include "Presentation/Assets/AbilityPresentationAsset.h"

void FDeliveryPresentation::CollectSoftPaths(TArray<FSoftObjectPath>& OutPaths) const
{
	if (Topology == EDeliveryTopology::None)
	{
		return;
	}
	if (!TrailVFX.IsNull())  OutPaths.AddUnique(TrailVFX.ToSoftObjectPath());
	if (!ImpactVFX.IsNull()) OutPaths.AddUnique(ImpactVFX.ToSoftObjectPath());
	if (!TrailSFX.IsNull())  OutPaths.AddUnique(TrailSFX.ToSoftObjectPath());
	if (!ImpactSFX.IsNull()) OutPaths.AddUnique(ImpactSFX.ToSoftObjectPath());
}

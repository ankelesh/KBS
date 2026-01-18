#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Services/TacGridMovementService.h"

void UGridSubsystem::RegisterManager(UGridDataManager* InDataManager)
{
	if (InDataManager)
	{
		DataManager = InDataManager;
		GridMovementService = NewObject<UTacGridMovementService>(this);
		GridMovementService->Initialize(InDataManager);
		GridTargetingService = NewObject<UTacGridTargetingService>(this);
		GridTargetingService->Initialize(InDataManager);
	}
}

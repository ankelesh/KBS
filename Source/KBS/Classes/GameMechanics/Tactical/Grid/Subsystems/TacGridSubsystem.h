#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TacGridSubsystem.generated.h"

namespace Tactical {

	class UGridDataManager;
	class UTacGridMovementService;
	class UTacGridTargetingService;
	UCLASS()
		class KBS_API UGridSubsystem : public UWorldSubsystem
	{
		GENERATED_BODY()
	public:
		UGridSubsystem() {}
		void RegisterManager(UGridDataManager* InDataManager);

		UTacGridMovementService* GetGridMovementService() { return GridMovementService; }
		UTacGridTargetingService* GetGridTargetingService() { return GridTargetingService; }

	private:

		UGridDataManager* DataManager;
		UTacGridMovementService* GridMovementService;
		UTacGridTargetingService* GridTargetingService;




	};
}; // namespace Tactical
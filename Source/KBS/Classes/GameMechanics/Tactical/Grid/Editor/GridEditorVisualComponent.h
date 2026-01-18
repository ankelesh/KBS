#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridEditorVisualComponent.generated.h"

namespace Tactical {
#if WITH_EDITOR

	class ATacBattleGrid;

	UCLASS()
		class KBS_API UGridEditorVisualComponent : public UActorComponent
	{
		GENERATED_BODY()
	public:
		UGridEditorVisualComponent();

		void DrawPreview();
		void DrawUnitPlacements();
		void DrawGridCells();

	private:
		ATacBattleGrid* GetGrid() const;
	};

#endif // WITH_EDITOR
}; // namespace Tactical
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeamUnitSlot.generated.h"

class UUnitDefinition;
class UTextBlock;
class UImage;
class UTeamGridPanel;

UCLASS(Blueprintable)
class KBS_API UTeamUnitSlot : public UUserWidget
{
	GENERATED_BODY()
public:
	void Setup(int32 InRow, int32 InCol, UTeamGridPanel* InParent);
	void SetOccupied(UUnitDefinition* Definition);
	void SetEmpty();
	void SetSecondaryCell(bool bIsSecondary);

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UTextBlock> UnitNameLabel;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UImage> SlotBackground;

	// BP hooks for visual state changes
	UFUNCTION(BlueprintImplementableEvent) void BP_OnSetOccupied(const FString& Name);
	UFUNCTION(BlueprintImplementableEvent) void BP_OnSetEmpty();
	UFUNCTION(BlueprintImplementableEvent) void BP_OnSetSecondary(bool bIsSecondary);
	UFUNCTION(BlueprintImplementableEvent) void BP_OnDragEnter(bool bIsValid);
	UFUNCTION(BlueprintImplementableEvent) void BP_OnDragLeave();

private:
	int32 SlotRow = 0;
	int32 SlotCol = 0;
	TWeakObjectPtr<UTeamGridPanel> ParentPanel;
};

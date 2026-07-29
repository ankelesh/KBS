#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitRosterSlot.generated.h"

class UUnitDefinition;
class UTextBlock;
class UImage;

UCLASS(Blueprintable)
class KBS_API UUnitRosterSlot : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetDefinition(UUnitDefinition* InDefinition);

	DECLARE_DELEGATE_OneParam(FOnRightClicked, UUnitDefinition*);
	FOnRightClicked OnRightClicked;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UTextBlock> UnitNameLabel;

private:
	UPROPERTY() TObjectPtr<UUnitDefinition> Definition;
};

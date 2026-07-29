#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitRosterPanel.generated.h"

class UUnitDefinition;
class UUnitRosterSlot;
class UScrollBox;

UCLASS(Blueprintable)
class KBS_API UUnitRosterPanel : public UUserWidget
{
	GENERATED_BODY()
public:
	// Populated manually in BP defaults or via code before NativeConstruct
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roster")
	TArray<TObjectPtr<UUnitDefinition>> UnitRoster;

	DECLARE_DELEGATE_OneParam(FOnUnitInfoRequested, UUnitDefinition*);
	FOnUnitInfoRequested OnUnitInfoRequested;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UScrollBox> RosterScrollBox;
	UPROPERTY(EditDefaultsOnly) TSubclassOf<UUnitRosterSlot> SlotClass;
};

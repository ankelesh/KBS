#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeamTable.generated.h"

class UBattleTeam;
class UUnitTeamSlot;
class UUniformGridPanel;

UCLASS(Blueprintable)
class KBS_API UTeamTable : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Team Table")
	void SetTeam(UBattleTeam* InTeam);

	UFUNCTION(BlueprintCallable, Category = "Team Table")
	void Clear();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UUniformGridPanel* UnitGrid;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UUnitTeamSlot> SlotClass;

private:
	void EmplaceSlot(UUnitTeamSlot* Slot, int32 Index);

	UPROPERTY()
	TObjectPtr<UBattleTeam> BoundTeam = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UUnitTeamSlot>> Slots;

	static constexpr int32 ColumnsPerRow = 2;
};

#pragma once
#include "CoreMinimal.h"
#include "Unit.h"
#include "LargeUnit.generated.h"

UCLASS()
class KBS_API ALargeUnit : public AUnit
{
	GENERATED_BODY()
public:
	virtual int32 GetCellSize() const override { return 2; }
	virtual bool IsMultiCell() const override { return true; }

	static FIntPoint GetSecondaryCell(int32 PrimaryRow, int32 PrimaryCol, bool bIsHorizontal, ETeamSide Team);
};

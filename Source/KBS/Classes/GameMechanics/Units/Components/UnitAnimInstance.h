#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "UnitAnimInstance.generated.h"

// Base class every unit AnimBP must be parented to, so unit-driven state (death, movement, ...)
// gets pushed from C++ with compile-time safety instead of via reflection.
UCLASS()
class KBS_API UUnitAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Unit")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Unit")
	bool bIsMoving = false;

	void SetIsDead(bool bDead) { bIsDead = bDead; }
	void SetIsMoving(bool bMoving) { bIsMoving = bMoving; }
};

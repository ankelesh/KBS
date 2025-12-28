#pragma once
#include "CoreMinimal.h"
#include "AbilityDisplayData.generated.h"
USTRUCT(BlueprintType)
struct KBS_API FAbilityDisplayData
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	FString AbilityName;
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	TObjectPtr<UTexture2D> Icon = nullptr;
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	int32 RemainingCharges = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	int32 MaxCharges = -1;
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	bool bCanExecuteThisTurn = false;
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	FString Description;
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	FString TargetingInfo;
	UPROPERTY(BlueprintReadWrite, Category = "Ability Display")
	bool bIsEmpty = true;
};

// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"

// Writes log messages of a single category to a dedicated file under ProjectLogDir.
class FTacCategoryLogger : public FOutputDevice
{
public:
	FTacCategoryLogger(FName InCategory, const FString& FileName);
	virtual ~FTacCategoryLogger() override;

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;
	virtual void TearDown() override;
	virtual bool CanBeUsedOnAnyThread() const override { return false; }

private:
	FName TrackedCategory;
	TUniquePtr<FArchive> FileWriter;
};

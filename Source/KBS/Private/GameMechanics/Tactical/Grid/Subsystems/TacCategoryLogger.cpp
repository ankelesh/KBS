// Fill out your copyright notice in the Description page of Project Settings.
#include "GameMechanics/Tactical/Grid/Subsystems/TacCategoryLogger.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Logging/LogVerbosity.h"

FTacCategoryLogger::FTacCategoryLogger(FName InCategory, const FString& FileName)
	: TrackedCategory(InCategory)
{
	const FString FilePath = FPaths::ProjectLogDir() / FileName;
	FileWriter.Reset(IFileManager::Get().CreateFileWriter(*FilePath, FILEWRITE_AllowRead));
}

FTacCategoryLogger::~FTacCategoryLogger()
{
	TearDown();
}

void FTacCategoryLogger::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	if (Category != TrackedCategory || !FileWriter)
	{
		return;
	}

	const FString Line = FString::Printf(
		TEXT("[%s][%s] %s\n"),
		*FDateTime::Now().ToString(TEXT("%H:%M:%S")),
		ToString(Verbosity),
		V
	);

	const FTCHARToUTF8 Utf8(*Line);
	FileWriter->Serialize(const_cast<ANSICHAR*>(Utf8.Get()), Utf8.Length());
	FileWriter->Flush();
}

void FTacCategoryLogger::TearDown()
{
	if (FileWriter)
	{
		FileWriter->Close();
		FileWriter.Reset();
	}
}

#include "KBSAssetToolsLibrary.h"
#include "KBSEditorToolsModule.h"
#include "KBSPropertyDump.h"
#include "KBSPropertyPath.h"
#include "Engine/Blueprint.h"
#include "FileHelpers.h"
#include "Logging/TokenizedMessage.h"
#include "Misc/DataValidation.h"
#include "Misc/PackageName.h"

namespace
{
	const TCHAR* WritableRoot = TEXT("/Game/_KBS/");

	/** Returns true when the op is valid; with bCommit it also performs the change. Summary describes the change. */
	using FMutationOp = TFunctionRef<bool(const FKBSResolvedProperty& Resolved, bool bCommit, FString& OutSummary, FString& OutError)>;

	FKBSToolResult Failure(const FString& Message)
	{
		FKBSToolResult Result;
		Result.Message = Message;
		return Result;
	}

	FKBSToolResult Success(const FString& Message, const FString& Value = FString())
	{
		FKBSToolResult Result;
		Result.bSuccess = true;
		Result.Message = Message;
		Result.Value = Value;
		return Result;
	}

	FString RefusalMessage(const FString& Path)
	{
		return FString::Printf(TEXT("REFUSED %s: writes are only allowed under %s"), *Path, WritableRoot);
	}

	UObject* LoadAsset(const FString& AssetPath, FString& OutError)
	{
		FString ObjectPath = AssetPath;
		if (!ObjectPath.Contains(TEXT(".")))
		{
			ObjectPath += TEXT(".") + FPackageName::GetShortName(ObjectPath);
		}
		UObject* Asset = StaticLoadObject(UObject::StaticClass(), nullptr, *ObjectPath, nullptr, LOAD_NoWarn);
		if (!Asset)
		{
			OutError = FString::Printf(TEXT("ERROR cannot load asset '%s'"), *AssetPath);
		}
		return Asset;
	}

	/** Blueprints resolve to their generated class default object, everything else to the asset itself. */
	UObject* LoadEditableObject(const FString& AssetPath, FString& OutError)
	{
		UObject* Asset = LoadAsset(AssetPath, OutError);
		if (!Asset)
		{
			return nullptr;
		}
		if (const UBlueprint* Blueprint = Cast<UBlueprint>(Asset))
		{
			if (!Blueprint->GeneratedClass)
			{
				OutError = FString::Printf(TEXT("ERROR blueprint '%s' has no generated class (compile it first)"), *AssetPath);
				return nullptr;
			}
			return Blueprint->GeneratedClass->GetDefaultObject();
		}
		return Asset;
	}

	FKBSToolResult RunMutation(const FString& AssetPath, const FString& PropertyPath, bool bApply, bool bSaveAsset,
		EPropertyChangeType::Type ChangeType, FMutationOp Op)
	{
		if (!UKBSAssetToolsLibrary::IsWritablePath(AssetPath))
		{
			return Failure(RefusalMessage(AssetPath));
		}

		FString Error;
		UObject* Root = LoadEditableObject(AssetPath, Error);
		if (!Root)
		{
			return Failure(Error);
		}
		UPackage* Package = Root->GetOutermost();
		if (!UKBSAssetToolsLibrary::IsWritablePath(Package->GetName()))
		{
			return Failure(RefusalMessage(Package->GetName()));
		}

		FKBSResolvedProperty Resolved;
		if (!KBSPropertyPath::Resolve(Root, PropertyPath, Resolved, Error))
		{
			return Failure(TEXT("ERROR ") + Error);
		}

		FString Summary;
		if (!Op(Resolved, false, Summary, Error))
		{
			return Failure(TEXT("ERROR ") + Error);
		}
		if (!bApply)
		{
			return Success(TEXT("WOULD ") + Summary);
		}

		Root->Modify();
		Resolved.Owner->Modify();
		KBSPropertyPath::NotifyPreChange(Resolved);
		FString CommitSummary;
		const bool bCommitted = Op(Resolved, true, CommitSummary, Error);
		KBSPropertyPath::NotifyPostChange(Resolved, ChangeType);
		if (!bCommitted)
		{
			return Failure(TEXT("ERROR ") + Error);
		}
		Package->MarkPackageDirty();

		FString Message = TEXT("APPLIED ") + Summary;
		if (bSaveAsset)
		{
			if (!UEditorLoadingAndSavingUtils::SavePackages({ Package }, false))
			{
				return Failure(FString::Printf(TEXT("ERROR %s was applied in memory but saving %s failed"), *Summary, *Package->GetName()));
			}
			Message += FString::Printf(TEXT(" (saved %s)"), *Package->GetName());
		}
		else
		{
			Message += FString::Printf(TEXT(" (unsaved: %s is dirty)"), *Package->GetName());
		}
		UE_LOG(LogKBSEditorTools, Log, TEXT("%s"), *Message);
		return Success(Message);
	}

	int32 FindSetIndex(FScriptSetHelper& Helper, const FProperty* ElementProperty, const void* Element)
	{
		for (int32 Index = 0, Remaining = Helper.Num(); Remaining > 0; ++Index)
		{
			if (Helper.IsValidIndex(Index))
			{
				--Remaining;
				if (ElementProperty->Identical(Helper.GetElementPtr(Index), Element))
				{
					return Index;
				}
			}
		}
		return INDEX_NONE;
	}

	int32 FindMapIndex(FScriptMapHelper& Helper, const FProperty* KeyProperty, const void* Key)
	{
		for (int32 Index = 0, Remaining = Helper.Num(); Remaining > 0; ++Index)
		{
			if (Helper.IsValidIndex(Index))
			{
				--Remaining;
				if (KeyProperty->Identical(Helper.GetKeyPtr(Index), Key))
				{
					return Index;
				}
			}
		}
		return INDEX_NONE;
	}

	template <typename TProperty>
	const TProperty* RequireContainer(const FKBSResolvedProperty& Resolved, const TCHAR* Kind, FString& OutError)
	{
		const TProperty* Container = CastField<TProperty>(Resolved.ValueProperty);
		if (!Container)
		{
			OutError = FString::Printf(TEXT("'%s' is a %s, expected %s"), *Resolved.ValueProperty->GetAuthoredName(), *Resolved.ValueProperty->GetCPPType(), Kind);
		}
		return Container;
	}

	FString DescribeSize(int32 Before, int32 After)
	{
		return FString::Printf(TEXT("(%d -> %d elements)"), Before, After);
	}
}

bool UKBSAssetToolsLibrary::IsWritablePath(const FString& PackagePath)
{
	FString Path = PackagePath.Replace(TEXT("\\"), TEXT("/"));
	int32 DotIndex;
	if (Path.FindChar(TEXT('.'), DotIndex))
	{
		Path.LeftInline(DotIndex);
	}
	TArray<FString> Parts;
	Path.ParseIntoArray(Parts, TEXT("/"));
	if (Parts.Contains(TEXT("..")))
	{
		return false;
	}
	Path.RemoveFromEnd(TEXT("/"));
	return (Path + TEXT("/")).StartsWith(WritableRoot, ESearchCase::IgnoreCase);
}

FKBSToolResult UKBSAssetToolsLibrary::GetPropertyText(const FString& AssetPath, const FString& PropertyPath)
{
	FString Error;
	UObject* Root = LoadEditableObject(AssetPath, Error);
	if (!Root)
	{
		return Failure(Error);
	}
	FKBSResolvedProperty Resolved;
	if (!KBSPropertyPath::Resolve(Root, PropertyPath, Resolved, Error))
	{
		return Failure(TEXT("ERROR ") + Error);
	}
	return Success(TEXT("OK"), KBSPropertyPath::ExportValue(Resolved.ValueProperty, Resolved.ValuePtr, Resolved.Owner));
}

FKBSToolResult UKBSAssetToolsLibrary::SetPropertyText(const FString& AssetPath, const FString& PropertyPath, const FString& ValueText, bool bApply, bool bSaveAsset)
{
	return RunMutation(AssetPath, PropertyPath, bApply, bSaveAsset, EPropertyChangeType::ValueSet,
		[&](const FKBSResolvedProperty& Resolved, bool bCommit, FString& OutSummary, FString& OutError)
		{
			FKBSScratchValue Scratch(Resolved.ValueProperty);
			Resolved.ValueProperty->CopyCompleteValue(Scratch.Get(), Resolved.ValuePtr);
			if (!KBSPropertyPath::ImportValue(Resolved.ValueProperty, Scratch.Get(), Resolved.Owner, ValueText, OutError))
			{
				return false;
			}
			OutSummary = FString::Printf(TEXT("set %s: %s -> %s"), *PropertyPath,
				*KBSPropertyPath::ExportValue(Resolved.ValueProperty, Resolved.ValuePtr, Resolved.Owner),
				*KBSPropertyPath::ExportValue(Resolved.ValueProperty, Scratch.Get(), Resolved.Owner));
			if (bCommit)
			{
				Resolved.ValueProperty->CopyCompleteValue(Resolved.ValuePtr, Scratch.Get());
			}
			return true;
		});
}

FKBSToolResult UKBSAssetToolsLibrary::GetContainerNum(const FString& AssetPath, const FString& PropertyPath)
{
	FString Error;
	UObject* Root = LoadEditableObject(AssetPath, Error);
	if (!Root)
	{
		return Failure(Error);
	}
	FKBSResolvedProperty Resolved;
	if (!KBSPropertyPath::Resolve(Root, PropertyPath, Resolved, Error))
	{
		return Failure(TEXT("ERROR ") + Error);
	}

	int32 Num;
	if (const FArrayProperty* Array = CastField<FArrayProperty>(Resolved.ValueProperty))
	{
		Num = FScriptArrayHelper(Array, Resolved.ValuePtr).Num();
	}
	else if (const FSetProperty* Set = CastField<FSetProperty>(Resolved.ValueProperty))
	{
		Num = FScriptSetHelper(Set, Resolved.ValuePtr).Num();
	}
	else if (const FMapProperty* Map = CastField<FMapProperty>(Resolved.ValueProperty))
	{
		Num = FScriptMapHelper(Map, Resolved.ValuePtr).Num();
	}
	else
	{
		return Failure(FString::Printf(TEXT("ERROR '%s' is not an array, set or map"), *PropertyPath));
	}
	return Success(TEXT("OK"), FString::FromInt(Num));
}

FKBSToolResult UKBSAssetToolsLibrary::ArrayAppend(const FString& AssetPath, const FString& PropertyPath, const FString& ElementText, bool bApply, bool bSaveAsset)
{
	return RunMutation(AssetPath, PropertyPath, bApply, bSaveAsset, EPropertyChangeType::ArrayAdd,
		[&](const FKBSResolvedProperty& Resolved, bool bCommit, FString& OutSummary, FString& OutError)
		{
			const FArrayProperty* Array = RequireContainer<FArrayProperty>(Resolved, TEXT("array"), OutError);
			if (!Array)
			{
				return false;
			}
			FScriptArrayHelper Helper(Array, Resolved.ValuePtr);
			FKBSScratchValue Scratch(Array->Inner);
			if (!KBSPropertyPath::ImportValue(Array->Inner, Scratch.Get(), Resolved.Owner, ElementText, OutError))
			{
				return false;
			}
			OutSummary = FString::Printf(TEXT("append to %s %s: %s"), *PropertyPath, *DescribeSize(Helper.Num(), Helper.Num() + 1),
				*KBSPropertyPath::ExportValue(Array->Inner, Scratch.Get(), Resolved.Owner));
			if (bCommit)
			{
				Array->Inner->CopyCompleteValue(Helper.GetRawPtr(Helper.AddValue()), Scratch.Get());
			}
			return true;
		});
}

FKBSToolResult UKBSAssetToolsLibrary::ArrayInsert(const FString& AssetPath, const FString& PropertyPath, int32 Index, const FString& ElementText, bool bApply, bool bSaveAsset)
{
	return RunMutation(AssetPath, PropertyPath, bApply, bSaveAsset, EPropertyChangeType::ArrayAdd,
		[&](const FKBSResolvedProperty& Resolved, bool bCommit, FString& OutSummary, FString& OutError)
		{
			const FArrayProperty* Array = RequireContainer<FArrayProperty>(Resolved, TEXT("array"), OutError);
			if (!Array)
			{
				return false;
			}
			FScriptArrayHelper Helper(Array, Resolved.ValuePtr);
			if (Index < 0 || Index > Helper.Num())
			{
				OutError = FString::Printf(TEXT("insert index %d out of range 0..%d"), Index, Helper.Num());
				return false;
			}
			FKBSScratchValue Scratch(Array->Inner);
			if (!KBSPropertyPath::ImportValue(Array->Inner, Scratch.Get(), Resolved.Owner, ElementText, OutError))
			{
				return false;
			}
			OutSummary = FString::Printf(TEXT("insert into %s at %d %s: %s"), *PropertyPath, Index, *DescribeSize(Helper.Num(), Helper.Num() + 1),
				*KBSPropertyPath::ExportValue(Array->Inner, Scratch.Get(), Resolved.Owner));
			if (bCommit)
			{
				Helper.InsertValues(Index);
				Array->Inner->CopyCompleteValue(Helper.GetRawPtr(Index), Scratch.Get());
			}
			return true;
		});
}

FKBSToolResult UKBSAssetToolsLibrary::ArrayRemoveAt(const FString& AssetPath, const FString& PropertyPath, int32 Index, bool bApply, bool bSaveAsset)
{
	return RunMutation(AssetPath, PropertyPath, bApply, bSaveAsset, EPropertyChangeType::ArrayRemove,
		[&](const FKBSResolvedProperty& Resolved, bool bCommit, FString& OutSummary, FString& OutError)
		{
			const FArrayProperty* Array = RequireContainer<FArrayProperty>(Resolved, TEXT("array"), OutError);
			if (!Array)
			{
				return false;
			}
			FScriptArrayHelper Helper(Array, Resolved.ValuePtr);
			if (!Helper.IsValidIndex(Index))
			{
				OutError = FString::Printf(TEXT("remove index %d out of range (size %d)"), Index, Helper.Num());
				return false;
			}
			OutSummary = FString::Printf(TEXT("remove %s[%d] %s: %s"), *PropertyPath, Index, *DescribeSize(Helper.Num(), Helper.Num() - 1),
				*KBSPropertyPath::ExportValue(Array->Inner, Helper.GetRawPtr(Index), Resolved.Owner));
			if (bCommit)
			{
				Helper.RemoveValues(Index);
			}
			return true;
		});
}

FKBSToolResult UKBSAssetToolsLibrary::ArrayClear(const FString& AssetPath, const FString& PropertyPath, bool bApply, bool bSaveAsset)
{
	return RunMutation(AssetPath, PropertyPath, bApply, bSaveAsset, EPropertyChangeType::ArrayClear,
		[&](const FKBSResolvedProperty& Resolved, bool bCommit, FString& OutSummary, FString& OutError)
		{
			const FArrayProperty* Array = RequireContainer<FArrayProperty>(Resolved, TEXT("array"), OutError);
			if (!Array)
			{
				return false;
			}
			FScriptArrayHelper Helper(Array, Resolved.ValuePtr);
			OutSummary = FString::Printf(TEXT("clear %s %s"), *PropertyPath, *DescribeSize(Helper.Num(), 0));
			if (bCommit)
			{
				Helper.EmptyValues();
			}
			return true;
		});
}

FKBSToolResult UKBSAssetToolsLibrary::SetAdd(const FString& AssetPath, const FString& PropertyPath, const FString& ElementText, bool bApply, bool bSaveAsset)
{
	return RunMutation(AssetPath, PropertyPath, bApply, bSaveAsset, EPropertyChangeType::ArrayAdd,
		[&](const FKBSResolvedProperty& Resolved, bool bCommit, FString& OutSummary, FString& OutError)
		{
			const FSetProperty* Set = RequireContainer<FSetProperty>(Resolved, TEXT("set"), OutError);
			if (!Set)
			{
				return false;
			}
			FScriptSetHelper Helper(Set, Resolved.ValuePtr);
			FKBSScratchValue Scratch(Set->ElementProp);
			if (!KBSPropertyPath::ImportValue(Set->ElementProp, Scratch.Get(), Resolved.Owner, ElementText, OutError))
			{
				return false;
			}
			const FString Exported = KBSPropertyPath::ExportValue(Set->ElementProp, Scratch.Get(), Resolved.Owner);
			if (FindSetIndex(Helper, Set->ElementProp, Scratch.Get()) != INDEX_NONE)
			{
				OutSummary = FString::Printf(TEXT("add to %s: %s already present (no change)"), *PropertyPath, *Exported);
				return true;
			}
			OutSummary = FString::Printf(TEXT("add to %s %s: %s"), *PropertyPath, *DescribeSize(Helper.Num(), Helper.Num() + 1), *Exported);
			if (bCommit)
			{
				Set->ElementProp->CopyCompleteValue(Helper.GetElementPtr(Helper.AddDefaultValue_Invalid_NeedsRehash()), Scratch.Get());
				Helper.Rehash();
			}
			return true;
		});
}

FKBSToolResult UKBSAssetToolsLibrary::SetRemove(const FString& AssetPath, const FString& PropertyPath, const FString& ElementText, bool bApply, bool bSaveAsset)
{
	return RunMutation(AssetPath, PropertyPath, bApply, bSaveAsset, EPropertyChangeType::ArrayRemove,
		[&](const FKBSResolvedProperty& Resolved, bool bCommit, FString& OutSummary, FString& OutError)
		{
			const FSetProperty* Set = RequireContainer<FSetProperty>(Resolved, TEXT("set"), OutError);
			if (!Set)
			{
				return false;
			}
			FScriptSetHelper Helper(Set, Resolved.ValuePtr);
			FKBSScratchValue Scratch(Set->ElementProp);
			if (!KBSPropertyPath::ImportValue(Set->ElementProp, Scratch.Get(), Resolved.Owner, ElementText, OutError))
			{
				return false;
			}
			const int32 Found = FindSetIndex(Helper, Set->ElementProp, Scratch.Get());
			if (Found == INDEX_NONE)
			{
				OutError = FString::Printf(TEXT("set '%s' does not contain '%s'"), *PropertyPath, *ElementText);
				return false;
			}
			OutSummary = FString::Printf(TEXT("remove from %s %s: %s"), *PropertyPath, *DescribeSize(Helper.Num(), Helper.Num() - 1),
				*KBSPropertyPath::ExportValue(Set->ElementProp, Scratch.Get(), Resolved.Owner));
			if (bCommit)
			{
				Helper.RemoveAt(Found);
			}
			return true;
		});
}

FKBSToolResult UKBSAssetToolsLibrary::MapSet(const FString& AssetPath, const FString& PropertyPath, const FString& KeyText, const FString& ValueText, bool bApply, bool bSaveAsset)
{
	return RunMutation(AssetPath, PropertyPath, bApply, bSaveAsset, EPropertyChangeType::ValueSet,
		[&](const FKBSResolvedProperty& Resolved, bool bCommit, FString& OutSummary, FString& OutError)
		{
			const FMapProperty* Map = RequireContainer<FMapProperty>(Resolved, TEXT("map"), OutError);
			if (!Map)
			{
				return false;
			}
			FScriptMapHelper Helper(Map, Resolved.ValuePtr);
			FKBSScratchValue ScratchKey(Map->KeyProp);
			if (!KBSPropertyPath::ImportValue(Map->KeyProp, ScratchKey.Get(), Resolved.Owner, KeyText, OutError))
			{
				return false;
			}
			const int32 Found = FindMapIndex(Helper, Map->KeyProp, ScratchKey.Get());
			FKBSScratchValue ScratchValue(Map->ValueProp);
			if (Found != INDEX_NONE)
			{
				Map->ValueProp->CopyCompleteValue(ScratchValue.Get(), Helper.GetValuePtr(Found));
			}
			if (!KBSPropertyPath::ImportValue(Map->ValueProp, ScratchValue.Get(), Resolved.Owner, ValueText, OutError))
			{
				return false;
			}

			const FString NewValue = KBSPropertyPath::ExportValue(Map->ValueProp, ScratchValue.Get(), Resolved.Owner);
			const FString Key = KBSPropertyPath::ExportValue(Map->KeyProp, ScratchKey.Get(), Resolved.Owner);
			if (Found != INDEX_NONE)
			{
				OutSummary = FString::Printf(TEXT("set %s[%s]: %s -> %s"), *PropertyPath, *Key,
					*KBSPropertyPath::ExportValue(Map->ValueProp, Helper.GetValuePtr(Found), Resolved.Owner), *NewValue);
			}
			else
			{
				OutSummary = FString::Printf(TEXT("add %s[%s] %s: %s"), *PropertyPath, *Key, *DescribeSize(Helper.Num(), Helper.Num() + 1), *NewValue);
			}
			if (bCommit)
			{
				if (Found != INDEX_NONE)
				{
					Map->ValueProp->CopyCompleteValue(Helper.GetValuePtr(Found), ScratchValue.Get());
				}
				else
				{
					const int32 NewIndex = Helper.AddDefaultValue_Invalid_NeedsRehash();
					Map->KeyProp->CopyCompleteValue(Helper.GetKeyPtr(NewIndex), ScratchKey.Get());
					Map->ValueProp->CopyCompleteValue(Helper.GetValuePtr(NewIndex), ScratchValue.Get());
					Helper.Rehash();
				}
			}
			return true;
		});
}

FKBSToolResult UKBSAssetToolsLibrary::MapRemove(const FString& AssetPath, const FString& PropertyPath, const FString& KeyText, bool bApply, bool bSaveAsset)
{
	return RunMutation(AssetPath, PropertyPath, bApply, bSaveAsset, EPropertyChangeType::ArrayRemove,
		[&](const FKBSResolvedProperty& Resolved, bool bCommit, FString& OutSummary, FString& OutError)
		{
			const FMapProperty* Map = RequireContainer<FMapProperty>(Resolved, TEXT("map"), OutError);
			if (!Map)
			{
				return false;
			}
			FScriptMapHelper Helper(Map, Resolved.ValuePtr);
			FKBSScratchValue ScratchKey(Map->KeyProp);
			if (!KBSPropertyPath::ImportValue(Map->KeyProp, ScratchKey.Get(), Resolved.Owner, KeyText, OutError))
			{
				return false;
			}
			const int32 Found = FindMapIndex(Helper, Map->KeyProp, ScratchKey.Get());
			if (Found == INDEX_NONE)
			{
				OutError = FString::Printf(TEXT("map '%s' has no key '%s'"), *PropertyPath, *KeyText);
				return false;
			}
			OutSummary = FString::Printf(TEXT("remove %s[%s] %s: %s"), *PropertyPath,
				*KBSPropertyPath::ExportValue(Map->KeyProp, ScratchKey.Get(), Resolved.Owner), *DescribeSize(Helper.Num(), Helper.Num() - 1),
				*KBSPropertyPath::ExportValue(Map->ValueProp, Helper.GetValuePtr(Found), Resolved.Owner));
			if (bCommit)
			{
				Helper.RemoveAt(Found);
			}
			return true;
		});
}

FKBSToolResult UKBSAssetToolsLibrary::DumpAssetJson(const FString& AssetPath, bool bIncludeInherited)
{
	FString Error;
	UObject* Root = LoadEditableObject(AssetPath, Error);
	if (!Root)
	{
		return Failure(Error);
	}
	return Success(TEXT("OK"), KBSPropertyDump::ToJson(Root, bIncludeInherited));
}

FKBSToolResult UKBSAssetToolsLibrary::DumpAssetText(const FString& AssetPath, bool bIncludeInherited)
{
	FString Error;
	UObject* Root = LoadEditableObject(AssetPath, Error);
	if (!Root)
	{
		return Failure(Error);
	}
	return Success(TEXT("OK"), KBSPropertyDump::ToText(Root, bIncludeInherited));
}

FKBSToolResult UKBSAssetToolsLibrary::ValidateAsset(const FString& AssetPath)
{
	FString Error;
	const UObject* Asset = LoadAsset(AssetPath, Error);
	if (!Asset)
	{
		return Failure(Error);
	}

	FDataValidationContext Context;
	const EDataValidationResult ValidationResult = Asset->IsDataValid(Context);

	TArray<FString> Lines;
	for (const FDataValidationContext::FIssue& Issue : Context.GetIssues())
	{
		const FText Text = Issue.TokenizedMessage.IsValid() ? Issue.TokenizedMessage->ToText() : Issue.Message;
		const TCHAR* Severity = Issue.Severity == EMessageSeverity::Error ? TEXT("Error")
			: Issue.Severity <= EMessageSeverity::Warning ? TEXT("Warning") : TEXT("Info");
		Lines.Add(FString::Printf(TEXT("[%s] %s"), Severity, *Text.ToString()));
	}

	const TCHAR* Verdict = ValidationResult == EDataValidationResult::Valid ? TEXT("Valid")
		: ValidationResult == EDataValidationResult::Invalid ? TEXT("Invalid") : TEXT("NotValidated");
	FKBSToolResult Result = Success(FString::Printf(TEXT("%s: %u error(s), %u warning(s)"), Verdict, Context.GetNumErrors(), Context.GetNumWarnings()),
		FString::Join(Lines, TEXT("\n")));
	Result.bSuccess = ValidationResult != EDataValidationResult::Invalid;
	return Result;
}

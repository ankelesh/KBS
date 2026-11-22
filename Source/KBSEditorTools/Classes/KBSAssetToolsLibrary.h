#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "KBSAssetToolsLibrary.generated.h"

USTRUCT(BlueprintType)
struct FKBSToolResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "KBS|EditorTools")
	bool bSuccess = false;

	/** "WOULD ..." for a dry run, "APPLIED ..." after a change, "REFUSED ..."/"ERROR ..." on failure. */
	UPROPERTY(BlueprintReadWrite, Category = "KBS|EditorTools")
	FString Message;

	/** Payload of read functions (property text, JSON, container size, validation issues). */
	UPROPERTY(BlueprintReadWrite, Category = "KBS|EditorTools")
	FString Value;
};

/**
 * Editor-only property reflection tools driven from Python (unreal.KBSAssetToolsLibrary).
 *
 * PropertyPath syntax: segments separated by '.', each optionally followed by subscripts:
 *   data.base_stats.accuracy_multiplier   struct members
 *   effects[2].magnitude                  array element
 *   Map["key"]                            map value by key (key given as UE text, quotes optional)
 * Names match the real UE name or its snake_case form, case-insensitive, underscores ignored.
 *
 * Mutating functions change nothing unless bApply is true and are limited to /Game/_KBS/.
 * Values and elements are UE text (e.g. "(A=1,B=\"x\")", "(1,2,3)", "/Game/_KBS/Foo.Foo").
 */
UCLASS()
class KBSEDITORTOOLS_API UKBSAssetToolsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The single scope guard: only packages under /Game/_KBS/ are writable. */
	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static bool IsWritablePath(const FString& PackagePath);

	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult GetPropertyText(const FString& AssetPath, const FString& PropertyPath);

	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult SetPropertyText(const FString& AssetPath, const FString& PropertyPath, const FString& ValueText, bool bApply, bool bSaveAsset);

	/** Sets an FInstancedStruct property to a new instance of StructPath (e.g. "/Script/KBS.UnitChargeComponentConfig"),
	 *  with FieldsText as plain struct text "(Entries=(...))" (empty = defaults). */
	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult SetInstancedStruct(const FString& AssetPath, const FString& PropertyPath, const FString& StructPath, const FString& FieldsText, bool bApply, bool bSaveAsset);

	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult GetContainerNum(const FString& AssetPath, const FString& PropertyPath);

	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult ArrayAppend(const FString& AssetPath, const FString& PropertyPath, const FString& ElementText, bool bApply, bool bSaveAsset);

	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult ArrayInsert(const FString& AssetPath, const FString& PropertyPath, int32 Index, const FString& ElementText, bool bApply, bool bSaveAsset);

	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult ArrayRemoveAt(const FString& AssetPath, const FString& PropertyPath, int32 Index, bool bApply, bool bSaveAsset);

	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult ArrayClear(const FString& AssetPath, const FString& PropertyPath, bool bApply, bool bSaveAsset);

	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult SetAdd(const FString& AssetPath, const FString& PropertyPath, const FString& ElementText, bool bApply, bool bSaveAsset);

	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult SetRemove(const FString& AssetPath, const FString& PropertyPath, const FString& ElementText, bool bApply, bool bSaveAsset);

	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult MapSet(const FString& AssetPath, const FString& PropertyPath, const FString& KeyText, const FString& ValueText, bool bApply, bool bSaveAsset);

	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult MapRemove(const FString& AssetPath, const FString& PropertyPath, const FString& KeyText, bool bApply, bool bSaveAsset);

	/** Pretty-printed JSON of all CPF_Edit properties. Blueprints dump their generated class CDO. */
	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult DumpAssetJson(const FString& AssetPath, bool bIncludeInherited = false);

	/** One "Name = ExportText" line per CPF_Edit property. */
	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult DumpAssetText(const FString& AssetPath, bool bIncludeInherited = false);

	/** Runs UObject::IsDataValid; Value holds one line per issue. */
	UFUNCTION(BlueprintCallable, Category = "KBS|EditorTools")
	static FKBSToolResult ValidateAsset(const FString& AssetPath);
};

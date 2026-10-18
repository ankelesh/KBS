#include "KBSPropertyDump.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/UnrealType.h"

namespace
{
	constexpr int32 MaxSubobjectDepth = 6;

	struct FDumpContext
	{
		const UObject* Root;
		bool bIncludeInherited;
	};

	bool IsEngineBase(const UStruct* Struct)
	{
		const FString PackageName = Struct->GetOutermost()->GetName();
		return PackageName.StartsWith(TEXT("/Script/Engine")) || PackageName.StartsWith(TEXT("/Script/CoreUObject"));
	}

	bool ShouldDump(const FProperty* Property, const FDumpContext& Context)
	{
		return Property->HasAnyPropertyFlags(CPF_Edit) && (Context.bIncludeInherited || !IsEngineBase(Property->GetOwnerStruct()));
	}

	FString ExportText(const FProperty* Property, const void* ValuePtr, UObject* Owner)
	{
		FString Text;
		Property->ExportText_Direct(Text, ValuePtr, ValuePtr, Owner, PPF_None);
		return Text;
	}

	TSharedPtr<FJsonValue> ValueToJson(const FProperty* Property, const void* ValuePtr, const FDumpContext& Context, int32 Depth);

	TSharedRef<FJsonObject> StructToJson(const UStruct* Struct, const void* StructPtr, const FDumpContext& Context, int32 Depth)
	{
		TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			if (!It->HasAnyPropertyFlags(CPF_Edit))
			{
				continue;
			}
			if (It->ArrayDim == 1)
			{
				Json->SetField(It->GetAuthoredName(), ValueToJson(*It, It->ContainerPtrToValuePtr<void>(StructPtr), Context, Depth));
				continue;
			}
			TArray<TSharedPtr<FJsonValue>> Elements;
			for (int32 Index = 0; Index < It->ArrayDim; ++Index)
			{
				Elements.Add(ValueToJson(*It, It->ContainerPtrToValuePtr<void>(StructPtr, Index), Context, Depth));
			}
			Json->SetArrayField(It->GetAuthoredName(), Elements);
		}
		return Json;
	}

	TSharedRef<FJsonObject> ObjectToJson(UObject* Object, const FDumpContext& Context, int32 Depth)
	{
		TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
		Json->SetStringField(TEXT("_class"), Object->GetClass()->GetPathName());
		for (TFieldIterator<FProperty> It(Object->GetClass()); It; ++It)
		{
			if (!ShouldDump(*It, Context))
			{
				continue;
			}
			if (It->ArrayDim == 1)
			{
				Json->SetField(It->GetAuthoredName(), ValueToJson(*It, It->ContainerPtrToValuePtr<void>(Object), Context, Depth));
				continue;
			}
			TArray<TSharedPtr<FJsonValue>> Elements;
			for (int32 Index = 0; Index < It->ArrayDim; ++Index)
			{
				Elements.Add(ValueToJson(*It, It->ContainerPtrToValuePtr<void>(Object, Index), Context, Depth));
			}
			Json->SetArrayField(It->GetAuthoredName(), Elements);
		}
		return Json;
	}

	bool HasEditableMembers(const UStruct* Struct)
	{
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			if (It->HasAnyPropertyFlags(CPF_Edit))
			{
				return true;
			}
		}
		return false;
	}

	TSharedPtr<FJsonValue> ValueToJson(const FProperty* Property, const void* ValuePtr, const FDumpContext& Context, int32 Depth)
	{
		if (const FBoolProperty* Bool = CastField<FBoolProperty>(Property))
		{
			return MakeShared<FJsonValueBoolean>(Bool->GetPropertyValue(ValuePtr));
		}
		if (const FEnumProperty* Enum = CastField<FEnumProperty>(Property))
		{
			const int64 Value = Enum->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePtr);
			return MakeShared<FJsonValueString>(Enum->GetEnum()->GetNameStringByValue(Value));
		}
		if (const FNumericProperty* Numeric = CastField<FNumericProperty>(Property))
		{
			if (const UEnum* Enum = Numeric->GetIntPropertyEnum())
			{
				return MakeShared<FJsonValueString>(Enum->GetNameStringByValue(Numeric->GetSignedIntPropertyValue(ValuePtr)));
			}
			const double Value = Numeric->IsFloatingPoint()
				? Numeric->GetFloatingPointPropertyValue(ValuePtr)
				: static_cast<double>(Numeric->GetSignedIntPropertyValue(ValuePtr));
			return MakeShared<FJsonValueNumber>(Value);
		}
		if (const FStrProperty* String = CastField<FStrProperty>(Property))
		{
			return MakeShared<FJsonValueString>(String->GetPropertyValue(ValuePtr));
		}
		if (const FNameProperty* Name = CastField<FNameProperty>(Property))
		{
			return MakeShared<FJsonValueString>(Name->GetPropertyValue(ValuePtr).ToString());
		}
		if (const FTextProperty* Text = CastField<FTextProperty>(Property))
		{
			return MakeShared<FJsonValueString>(Text->GetPropertyValue(ValuePtr).ToString());
		}
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			if (!HasEditableMembers(StructProperty->Struct))
			{
				return MakeShared<FJsonValueString>(ExportText(Property, ValuePtr, nullptr));
			}
			return MakeShared<FJsonValueObject>(StructToJson(StructProperty->Struct, ValuePtr, Context, Depth));
		}
		if (const FArrayProperty* Array = CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper Helper(Array, ValuePtr);
			TArray<TSharedPtr<FJsonValue>> Elements;
			for (int32 Index = 0; Index < Helper.Num(); ++Index)
			{
				Elements.Add(ValueToJson(Array->Inner, Helper.GetRawPtr(Index), Context, Depth));
			}
			return MakeShared<FJsonValueArray>(Elements);
		}
		if (const FSetProperty* Set = CastField<FSetProperty>(Property))
		{
			FScriptSetHelper Helper(Set, ValuePtr);
			TArray<TSharedPtr<FJsonValue>> Elements;
			for (int32 Index = 0, Remaining = Helper.Num(); Remaining > 0; ++Index)
			{
				if (Helper.IsValidIndex(Index))
				{
					--Remaining;
					Elements.Add(ValueToJson(Set->ElementProp, Helper.GetElementPtr(Index), Context, Depth));
				}
			}
			return MakeShared<FJsonValueArray>(Elements);
		}
		if (const FMapProperty* Map = CastField<FMapProperty>(Property))
		{
			FScriptMapHelper Helper(Map, ValuePtr);
			TSharedRef<FJsonObject> Entries = MakeShared<FJsonObject>();
			for (int32 Index = 0, Remaining = Helper.Num(); Remaining > 0; ++Index)
			{
				if (Helper.IsValidIndex(Index))
				{
					--Remaining;
					Entries->SetField(ExportText(Map->KeyProp, Helper.GetKeyPtr(Index), nullptr), ValueToJson(Map->ValueProp, Helper.GetValuePtr(Index), Context, Depth));
				}
			}
			return MakeShared<FJsonValueObject>(Entries);
		}
		if (Property->IsA<FSoftObjectProperty>())
		{
			const FSoftObjectPtr& Reference = *static_cast<const FSoftObjectPtr*>(ValuePtr);
			if (Reference.IsNull())
			{
				return MakeShared<FJsonValueNull>();
			}
			return MakeShared<FJsonValueString>(Reference.ToString());
		}
		if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
		{
			UObject* Object = ObjectProperty->GetObjectPropertyValue(ValuePtr);
			if (!Object)
			{
				return MakeShared<FJsonValueNull>();
			}
			if (Object->IsIn(Context.Root) && Depth < MaxSubobjectDepth)
			{
				return MakeShared<FJsonValueObject>(ObjectToJson(Object, Context, Depth + 1));
			}
			return MakeShared<FJsonValueString>(Object->GetPathName());
		}
		return MakeShared<FJsonValueString>(ExportText(Property, ValuePtr, nullptr));
	}
}

FString KBSPropertyDump::ToJson(UObject* Object, bool bIncludeInherited)
{
	const FDumpContext Context{ Object, bIncludeInherited };
	const TSharedRef<FJsonObject> Json = ObjectToJson(Object, Context, 0);
	Json->SetStringField(TEXT("_asset"), Object->GetOutermost()->GetName());

	FString Output;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Output);
	FJsonSerializer::Serialize(Json, Writer);
	return Output;
}

FString KBSPropertyDump::ToText(UObject* Object, bool bIncludeInherited)
{
	const FDumpContext Context{ Object, bIncludeInherited };
	FString Output = FString::Printf(TEXT("== %s (%s)"), *Object->GetOutermost()->GetName(), *Object->GetClass()->GetName());
	for (TFieldIterator<FProperty> It(Object->GetClass()); It; ++It)
	{
		if (ShouldDump(*It, Context))
		{
			Output += FString::Printf(TEXT("\n  %s = %s"), *It->GetAuthoredName(), *ExportText(*It, It->ContainerPtrToValuePtr<void>(Object), Object));
		}
	}
	return Output;
}

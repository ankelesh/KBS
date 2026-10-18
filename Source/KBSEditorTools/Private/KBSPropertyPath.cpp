#include "KBSPropertyPath.h"
#include "Misc/StringOutputDevice.h"

namespace
{
	struct FPathSegment
	{
		FString Name;
		TArray<FString> Subscripts;
	};

	FString Normalize(const FString& Name)
	{
		FString Result;
		for (const TCHAR Char : Name)
		{
			if (Char != TEXT('_') && !FChar::IsWhitespace(Char))
			{
				Result.AppendChar(FChar::ToLower(Char));
			}
		}
		return Result;
	}

	FString UnquoteSubscript(FString Subscript)
	{
		Subscript.TrimStartAndEndInline();
		if (Subscript.Len() >= 2 && Subscript.StartsWith(TEXT("\"")) && Subscript.EndsWith(TEXT("\"")))
		{
			Subscript = Subscript.Mid(1, Subscript.Len() - 2).Replace(TEXT("\\\""), TEXT("\""));
		}
		return Subscript;
	}

	bool ParsePath(const FString& Path, TArray<FPathSegment>& OutSegments, FString& OutError)
	{
		const int32 Length = Path.Len();
		int32 Cursor = 0;
		while (Cursor < Length)
		{
			FPathSegment Segment;
			while (Cursor < Length && Path[Cursor] != TEXT('.') && Path[Cursor] != TEXT('['))
			{
				Segment.Name.AppendChar(Path[Cursor++]);
			}
			Segment.Name.TrimStartAndEndInline();
			if (Segment.Name.IsEmpty())
			{
				OutError = FString::Printf(TEXT("empty property name in path '%s'"), *Path);
				return false;
			}

			while (Cursor < Length && Path[Cursor] == TEXT('['))
			{
				++Cursor;
				FString Subscript;
				int32 Depth = 0;
				bool bQuoted = false;
				bool bClosed = false;
				for (; Cursor < Length; ++Cursor)
				{
					const TCHAR Char = Path[Cursor];
					if (bQuoted)
					{
						if (Char == TEXT('\\') && Cursor + 1 < Length)
						{
							Subscript.AppendChar(Char);
							Subscript.AppendChar(Path[++Cursor]);
							continue;
						}
						bQuoted = Char != TEXT('"');
					}
					else if (Char == TEXT('"'))
					{
						bQuoted = true;
					}
					else if (Char == TEXT('(') || Char == TEXT('['))
					{
						++Depth;
					}
					else if (Char == TEXT(')'))
					{
						--Depth;
					}
					else if (Char == TEXT(']'))
					{
						if (Depth == 0)
						{
							bClosed = true;
							++Cursor;
							break;
						}
						--Depth;
					}
					Subscript.AppendChar(Char);
				}
				if (!bClosed)
				{
					OutError = FString::Printf(TEXT("unterminated '[' in path '%s'"), *Path);
					return false;
				}
				Segment.Subscripts.Add(UnquoteSubscript(Subscript));
			}

			if (Cursor < Length)
			{
				if (Path[Cursor] != TEXT('.'))
				{
					OutError = FString::Printf(TEXT("unexpected '%c' in path '%s'"), Path[Cursor], *Path);
					return false;
				}
				++Cursor;
			}
			OutSegments.Add(MoveTemp(Segment));
		}
		if (OutSegments.IsEmpty())
		{
			OutError = TEXT("property path is empty");
			return false;
		}
		return true;
	}

	FProperty* FindProperty(const UStruct* Struct, const FString& Name)
	{
		const FString Wanted = Normalize(Name);
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			const FString Candidate = Normalize(It->GetAuthoredName());
			const bool bBoolWithoutPrefix = It->IsA<FBoolProperty>() && Candidate == TEXT("b") + Wanted;
			if (Candidate == Wanted || bBoolWithoutPrefix)
			{
				return *It;
			}
		}
		return nullptr;
	}

	int32 FindMapIndexByKeyText(const FMapProperty* Map, FScriptMapHelper& Helper, UObject* Owner, const FString& KeyText, FString& OutError)
	{
		FKBSScratchValue ScratchKey(Map->KeyProp);
		if (!KBSPropertyPath::ImportValue(Map->KeyProp, ScratchKey.Get(), Owner, KeyText, OutError))
		{
			return INDEX_NONE;
		}
		for (int32 Index = 0, Remaining = Helper.Num(); Remaining > 0; ++Index)
		{
			if (Helper.IsValidIndex(Index))
			{
				--Remaining;
				if (Map->KeyProp->Identical(Helper.GetKeyPtr(Index), ScratchKey.Get()))
				{
					return Index;
				}
			}
		}
		OutError = FString::Printf(TEXT("map '%s' has no key '%s'"), *Map->GetAuthoredName(), *KeyText);
		return INDEX_NONE;
	}

	void BuildChain(const FKBSResolvedProperty& Resolved, FEditPropertyChain& OutChain)
	{
		for (FProperty* Property : Resolved.Chain)
		{
			OutChain.AddTail(Property);
		}
		OutChain.SetActivePropertyNode(Resolved.Chain.Last());
		OutChain.SetActiveMemberPropertyNode(Resolved.Chain[0]);
	}
}

FKBSScratchValue::FKBSScratchValue(const FProperty* InProperty)
	: Property(InProperty)
	, Memory(FMemory::Malloc(InProperty->GetSize(), InProperty->GetMinAlignment()))
{
	Property->InitializeValue(Memory);
}

FKBSScratchValue::~FKBSScratchValue()
{
	Property->DestroyValue(Memory);
	FMemory::Free(Memory);
}

bool KBSPropertyPath::Resolve(UObject* Root, const FString& Path, FKBSResolvedProperty& OutResolved, FString& OutError)
{
	checkf(Root, TEXT("Resolve requires a root object"));
	TArray<FPathSegment> Segments;
	if (!ParsePath(Path, Segments, OutError))
	{
		return false;
	}

	const UStruct* Struct = Root->GetClass();
	void* Container = Root;
	OutResolved = FKBSResolvedProperty();
	OutResolved.Owner = Root;

	for (int32 SegmentIndex = 0; SegmentIndex < Segments.Num(); ++SegmentIndex)
	{
		const FPathSegment& Segment = Segments[SegmentIndex];
		FProperty* Property = FindProperty(Struct, Segment.Name);
		if (!Property)
		{
			OutError = FString::Printf(TEXT("property '%s' not found on %s"), *Segment.Name, *Struct->GetName());
			return false;
		}
		OutResolved.Chain.Add(Property);

		FProperty* Current = Property;
		void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Container);
		for (const FString& Subscript : Segment.Subscripts)
		{
			if (const FArrayProperty* Array = CastField<FArrayProperty>(Current))
			{
				FScriptArrayHelper Helper(Array, ValuePtr);
				if (!Subscript.IsNumeric() || !Helper.IsValidIndex(FCString::Atoi(*Subscript)))
				{
					OutError = FString::Printf(TEXT("index '%s' is invalid for array '%s' (size %d)"), *Subscript, *Segment.Name, Helper.Num());
					return false;
				}
				ValuePtr = Helper.GetRawPtr(FCString::Atoi(*Subscript));
				Current = Array->Inner;
			}
			else if (const FMapProperty* Map = CastField<FMapProperty>(Current))
			{
				FScriptMapHelper Helper(Map, ValuePtr);
				const int32 MapIndex = FindMapIndexByKeyText(Map, Helper, OutResolved.Owner, Subscript, OutError);
				if (MapIndex == INDEX_NONE)
				{
					return false;
				}
				ValuePtr = Helper.GetValuePtr(MapIndex);
				Current = Map->ValueProp;
			}
			else
			{
				OutError = FString::Printf(TEXT("'%s' is neither an array nor a map, cannot subscript with [%s]"), *Segment.Name, *Subscript);
				return false;
			}
		}

		if (SegmentIndex == Segments.Num() - 1)
		{
			OutResolved.ValueProperty = Current;
			OutResolved.ValuePtr = ValuePtr;
			return true;
		}

		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Current))
		{
			Struct = StructProperty->Struct;
			Container = ValuePtr;
		}
		else if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Current))
		{
			UObject* Inner = ObjectProperty->GetObjectPropertyValue(ValuePtr);
			if (!Inner)
			{
				OutError = FString::Printf(TEXT("object reference '%s' is null, cannot descend"), *Segment.Name);
				return false;
			}
			Struct = Inner->GetClass();
			Container = Inner;
			OutResolved.Owner = Inner;
			OutResolved.Chain.Reset();
		}
		else
		{
			OutError = FString::Printf(TEXT("'%s' is not a struct or object reference, cannot descend"), *Segment.Name);
			return false;
		}
	}
	return false;
}

bool KBSPropertyPath::ImportValue(const FProperty* Property, void* ValuePtr, UObject* Owner, const FString& Text, FString& OutError)
{
	FStringOutputDevice Errors;
	const TCHAR* End = Property->ImportText_Direct(*Text, ValuePtr, Owner, PPF_None, &Errors);
	if (!End)
	{
		OutError = FString::Printf(TEXT("cannot import '%s' as %s"), *Text, *Property->GetCPPType());
		if (!Errors.IsEmpty())
		{
			OutError += FString::Printf(TEXT(" (%s)"), *Errors.TrimStartAndEnd());
		}
		return false;
	}
	while (FChar::IsWhitespace(*End))
	{
		++End;
	}
	if (*End != TEXT('\0'))
	{
		OutError = FString::Printf(TEXT("trailing text '%s' after importing %s"), End, *Property->GetCPPType());
		return false;
	}
	return true;
}

FString KBSPropertyPath::ExportValue(const FProperty* Property, const void* ValuePtr, UObject* Owner)
{
	FString Text;
	Property->ExportText_Direct(Text, ValuePtr, ValuePtr, Owner, PPF_None);
	return Text;
}

void KBSPropertyPath::NotifyPreChange(const FKBSResolvedProperty& Resolved)
{
	FEditPropertyChain Chain;
	BuildChain(Resolved, Chain);
	Resolved.Owner->PreEditChange(Chain);
}

void KBSPropertyPath::NotifyPostChange(const FKBSResolvedProperty& Resolved, EPropertyChangeType::Type ChangeType)
{
	FEditPropertyChain Chain;
	BuildChain(Resolved, Chain);
	FPropertyChangedEvent Event(Resolved.Chain.Last(), ChangeType);
	Event.SetActiveMemberProperty(Resolved.Chain[0]);
	FPropertyChangedChainEvent ChainEvent(Chain, Event);
	Resolved.Owner->PostEditChangeChainProperty(ChainEvent);
}

#pragma once
#include "CoreMinimal.h"
#include "UObject/UnrealType.h"

/** Temporary, initialized storage for one value of a property; used to validate text before touching real data. */
class FKBSScratchValue
{
public:
	explicit FKBSScratchValue(const FProperty* InProperty);
	~FKBSScratchValue();
	FKBSScratchValue(const FKBSScratchValue&) = delete;
	FKBSScratchValue& operator=(const FKBSScratchValue&) = delete;

	void* Get() const { return Memory; }

private:
	const FProperty* Property;
	void* Memory;
};

struct FKBSResolvedProperty
{
	/** Innermost object that owns the value (the asset, or an instanced subobject on the way). */
	UObject* Owner = nullptr;
	/** Property describing the memory at ValuePtr (an array's Inner / a map's ValueProp after a subscript). */
	FProperty* ValueProperty = nullptr;
	void* ValuePtr = nullptr;
	/** Named properties walked from Owner down to the value, used to build change chains. */
	TArray<FProperty*> Chain;
};

namespace KBSPropertyPath
{
	bool Resolve(UObject* Root, const FString& Path, FKBSResolvedProperty& OutResolved, FString& OutError);

	bool ImportValue(const FProperty* Property, void* ValuePtr, UObject* Owner, const FString& Text, FString& OutError);
	FString ExportValue(const FProperty* Property, const void* ValuePtr, UObject* Owner);

	void NotifyPreChange(const FKBSResolvedProperty& Resolved);
	void NotifyPostChange(const FKBSResolvedProperty& Resolved, EPropertyChangeType::Type ChangeType);
}

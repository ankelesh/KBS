#pragma once
#include "CoreMinimal.h"

namespace KBSPropertyDump
{
	/** Pretty-printed JSON of all CPF_Edit properties; instanced subobjects are nested, other references are path strings. */
	FString ToJson(UObject* Object, bool bIncludeInherited);

	/** "Name = ExportText" per CPF_Edit property. */
	FString ToText(UObject* Object, bool bIncludeInherited);
}
